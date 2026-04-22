# Mini DBMS Server - MVP 검토 및 피드백 (CYB)

> 구현 목표: HTTP POST /query 단일 엔드포인트로 INSERT/SELECT 지원하는 인메모리 DBMS

---

## 1. MVP 범위 판단

| 항목 | 구현 여부 | 평가 |
|------|-----------|------|
| POST /query 단일 엔드포인트 | ✅ | MVP에 적합 |
| INSERT / SELECT | ✅ | MVP에 적합 |
| WHERE (id, name, age) | ✅ | MVP 범위 내 |
| DELETE / UPDATE | ❌ 미구현 | MVP scope에서 의도적 제외, 적절 |
| KEEP-ALIVE | ❌ 미구현 | Connection: close 방식, MVP 수준으로 수용 가능 |
| 인덱스 (B+ 트리) | ✅ id PK | MVP치고는 오히려 과구현 |
| 멀티스레드 처리 | ✅ 스레드 풀 | 적절 |
| Graceful shutdown | ✅ | 잘 구현됨 |
| Docker 지원 | ✅ | 적절 |

---

## 2. 발견된 문제점 및 개선 제안

### [이슈 1] 뮤텍스 범위가 너무 넓다 — `api.c:124~163`

**현재 코드 흐름:**
```c
pthread_mutex_lock(db_mutex);
sql_result = sql_execute(table, sql);     // DB 접근 (락 필요)
json_buffer_init(&buffer, 256);          // 메모리 할당 (락 불필요)
api_build_success_response(...);         // JSON 직렬화 (락 불필요)
pthread_mutex_unlock(db_mutex);
```

**문제**: JSON 직렬화는 `table`에 접근하지 않는다. 뮤텍스를 잡은 채로 수행할 필요가 없다.  
워커 4개가 동시에 요청을 받아도 실질적으로 직렬화되어 처리된다.

**제안 수정:**
```c
pthread_mutex_lock(db_mutex);
sql_result = sql_execute(table, sql);
pthread_mutex_unlock(db_mutex);           // 여기서 즉시 해제

// 락 없이 JSON 직렬화
json_buffer_init(&buffer, 256);
api_build_success_response(&sql_result, &buffer);
sql_result_destroy(&sql_result);          // records[]도 여기서 해제
```

> 단, `sql_result.records`가 table 내부 포인터를 참조하는지 복사본인지 확인 필요.  
> `table.c`의 `table_copy_records()`를 보면 별도 malloc으로 복사하므로 안전하게 분리 가능.

---

### [이슈 2] B+ 트리를 범위 검색에 활용하지 않는다 — `table.c`

**현재:**
```
WHERE id = 1   → bptree_search()              → O(log n) ✅
WHERE id >= 2  → table_find_by_id_condition() → 전체 rows[] 선형 스캔 → O(n) ⚠️
```

`bptree.c`의 리프 노드는 `next` 포인터로 연결리스트를 구성하고 있어 범위 스캔이 가능하다.  
그러나 `table.c`는 이를 활용하지 않고 `rows[]` 배열을 처음부터 끝까지 순회한다.

**현 시점 판단**: 데이터가 소규모라면 문제없음. MVP scope 안. 단, B+ 트리를 도입한 목적과 모순.

**개선 방향 (선택)**:
```c
// bptree_find_leaf(tree, min_id) → leaf부터 next로 순회
// bptree에 range_scan 함수 추가 제안
```

---

### [이슈 3] 소켓 읽기 타임아웃 없음 — `http.c`

**현재**: `recv()` 호출 시 SO_RCVTIMEO 미설정.

```
느린 클라이언트가 header를 천천히 보내면
→ 워커 스레드 1개가 recv()에서 무기한 블로킹
→ 4개 워커가 모두 잡히면 queue full → 503
```

**제안**: `accept()` 직후에 타임아웃 설정.
```c
struct timeval tv = { .tv_sec = 5, .tv_usec = 0 };
setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
```

---

### [이슈 4] `src/` 디렉터리에 소스 파일 없이 `.o` 파일만 존재

```
src/
├── concurrency/job_queue.o
├── concurrency/thread_pool.o
├── db/db_adapter.o
├── main.o
└── server/server.o
```

대응하는 `.c` 파일이 없다. 이전 아키텍처 실험의 잔재로 보인다.  
빌드에 포함되지 않으므로 기능에 영향은 없지만, git 정리가 필요하다.

```bash
git rm --cached src/concurrency/job_queue.o src/concurrency/thread_pool.o \
  src/db/db_adapter.o src/main.o src/server/server.o
```

---

### [이슈 5] `sql_execute`의 시도-파싱 방식 — `sql.c:491~499`

```c
result = sql_execute_insert(table, input);
if (result.status != SQL_STATUS_SYNTAX_ERROR || result.error_message[0] != '\0') {
    return result;  // INSERT로 확정
}
result = sql_execute_select(table, input);  // INSERT 아니면 SELECT 시도
```

INSERT를 먼저 시도하고 실패하면 SELECT를 시도하는 방식이다.  
"에러 메시지가 없는 SYNTAX_ERROR"라는 암묵적 신호를 구분 플래그로 쓰는 건 취약하다.

**제안**: 첫 키워드를 먼저 읽어 분기하는 방식이 더 명확하다.
```c
sql_skip_spaces(&cursor);
if (sql_match_keyword(&cursor, "INSERT")) {
    return sql_execute_insert(table, input);
} else if (sql_match_keyword(&cursor, "SELECT")) {
    return sql_execute_select(table, input);
} else {
    sql_set_syntax_error(&result, cursor);
    return result;
}
```

---

### [이슈 6] `HttpRequest`가 스택에 4KB 이상 할당됨 — `server.c:118~149`

```c
static void server_handle_client(void *context, int client_fd) {
    HttpRequest request;       // char body[4097] + 헤더 필드 포함 ≈ 4.2 KB 스택
    // ...
```

`http.c`에도 `char buffer[8192]`가 있어서 워커 스레드당 실제 스택 사용량은 약 12KB+.  
4 스레드 기준 총 ~50KB. 기본 스레드 스택 크기(수 MB)에 비하면 문제없지만,  
스택 오버플로우 가능성이 있는 임베디드 환경에서는 heap 할당이 더 안전하다.

---

## 3. 잘 구현된 부분

| 항목 | 위치 | 설명 |
|------|------|------|
| SO_REUSEADDR 설정 | `server.c:48` | 빠른 재시작 지원, 실수 없이 챙김 |
| EINTR 재시도 | `server.c:218`, `http.c:18~30` | 시그널 인터럽트 시 재시도 패턴 |
| 역순 정리 (cleanup) | `server_create()` | 초기화 실패 시 이미 할당된 리소스를 역순으로 해제 |
| JSON 문자 이스케이핑 | `json_util.c:108~175` | `\\`, `\"`, `\n`, `\r`, `\t`, 제어문자 `\u00XX` 모두 처리 |
| API 에러 구조 분리 | `server.c` vs `api.c` | HTTP 레벨 에러와 SQL 레벨 에러를 다른 경로로 처리 |
| 워커 종료 조건 | `thread_pool.c:18` | `stop_requested && size==0` 조건으로 진행 중인 작업 완료 후 종료 |
| `strtoul` 포트 검증 | `server_main.c:40~44` | end_ptr 검사로 "abc" 같은 잘못된 포트 차단 |

---

*작성일: 2026-04-22*
