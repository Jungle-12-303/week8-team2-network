# 수석 엔지니어 평가: 미니 DBMS API 서버 계획 검토

> **작성 일시**: 2026-04-21
> **검토자**: 시스템 프로그래밍 & C 멀티스레드 전문가
> **검토 기준**: 수요일 하루 프로젝트 완성 가능성

---

## 1. 총평

이 계획은 **기본 구조는 건전하지만, 실제 구현 난이도를 20~30% 과소평가**하고 있다.
특히 **HTTP 파싱, JSON 메모리 관리, graceful shutdown 처리에서 치명적 구현 누락**이 있으며,
테스트 시나리오도 실패 확률이 높은 부분이 있다.

**종합 평가: 70점 (합격선 75점)**

| 영역 | 현황 | 위험도 |
|------|------|--------|
| 요구사항 충족 | ✅ 명시적으로 모두 포함 | 낮음 |
| 아키텍처 골격 | ✅ 합리적 계층 분리 | 낮음 |
| **HTTP 파싱** | ⚠️ 복잡성 과소평가 | **🔴 치명적** |
| **SQL 오류 처리** | ❌ 스펙 불명확 | **🔴 치명적** |
| **메모리 관리** | ⚠️ 고정 vs 동적 미결정 | **🟠 높음** |
| **Worker Shutdown** | ❌ 신호 메커니즘 없음 | **🟠 높음** |
| 테스트 시나리오 | ⚠️ id 충돌 위험 | 🟡 중간 |
| 코드 주석 계획 | ✅ 명확함 | 낮음 |

---

## 2. 과제 요구사항 충족 여부

### 현황: ✅ **체크리스트 상으로는 모두 충족**

- ✅ 외부 클라이언트 API (`POST /query`)
- ✅ Thread Pool (worker 4개, queue 64개)
- ✅ 병렬 처리 (worker가 queue에서 꺼냄)
- ✅ SQL 처리기 & B+Tree 재사용 (`sql_execute()` 호출)
- ✅ C 언어 구현
- ✅ 단위 테스트 + API 기능 테스트 + 동시성 테스트

### 문제점: ❌ **명시되지 않은 요구사항 해석의 혼동**

#### 문제 1: "외부 클라이언트가 DBMS 기능을 사용할 수 있어야 함"
```
계획: POST /query에 SQL을 text/plain으로 받음
평가: ✅ 합격 (간단하고 명확)
```

#### 문제 2: "구현한 API를 통해" ← API 응답이 **꼭 JSON이어야 하는가?**
```
계획: HTTP 200 OK + JSON body
문제:
  - guideLine에서 "API"라고 했지만, 응답 형식은 미명시
  - 03-api-contract.md는 JSON으로 정의했지만,
    실제로 테스트할 때는 JSON parsing이 필요
  - 단순 text 응답으로 충분할 수도 있음
평가: ⚠️ 스펙은 명확하지만, 구현 복잡도가 높음
```

#### 문제 3: "테스트와 데모 가능해야 함"
```
계획: curl로 테스트, README 기반 발표
평가: ✅ 좋음, 하지만 실패 시나리오가 많음 (후술)
```

**종합**: 요구사항은 **형식적으로 모두 충족**하지만, 실제 구현의 복잡도는 계획에 반영되지 않았다.

---

## 3. 최소 구현 관점에서의 적절성

### 현황: ⚠️ **범위는 적절하나, 난이도 예측이 낙관적**

### 3.1 범위 검증

#### MVP는 충분히 최소화되었는가?
```
✅ 사용자 테이블 고정
✅ INSERT / SELECT만 지원
✅ 단일 엔드포인트 POST /query
✅ 메모리 기반 저장소 (영속성 제외)
✅ 인증 / TLS / transaction 제외
```

**평가**: ✅ 범위 설정은 적절

#### 하지만 구현 난이도를 저평가한 부분들

**Level 1 (초급, 구현 용이)**
- ✅ 기존 SQL 처리기 호출
- ✅ Thread pool 생성 / join
- ✅ Mutex lock/unlock
- ✅ Accept loop 기본 구조

**Level 2 (중급, 주의 필요)**
- ⚠️ HTTP 요청 라인 파싱
- ⚠️ Content-Length 기반 body 읽기
- ⚠️ JSON 문자열 수작업 생성
- ⚠️ HTTP 응답 포맷 작성

**Level 3 (고급, 리스크 높음)** ← 계획에서 과소평가된 부분
- 🔴 부분 수신 (partial read) 처리
- 🔴 SQL 오류 메시지를 JSON에 담기
- 🔴 버퍼 오버플로우 방지
- 🔴 Worker shutdown 신호 처리
- 🔴 Queue full 시 503 응답 생성

### 3.2 하루 구현 가능성

| 항목 | 예상 시간 | 난이도 | 리스크 |
|------|---------|--------|--------|
| 기본 socket + accept loop | 30분 | 쉬움 | 낮음 |
| HTTP request line 파싱 | 45분 | 중간 | 중간 |
| Content-Length 기반 body 읽기 | 60분 | 중간 | **높음** |
| sql_execute() 호출 | 15분 | 쉬움 | 낮음 |
| INSERT 응답 JSON | 30분 | 중간 | 낮음 |
| SELECT 응답 JSON (배열) | 45분 | 중간 | **높음** |
| Worker thread + queue | 90분 | 어려움 | 낮음 |
| DB mutex 보호 | 30분 | 중간 | **높음** |
| SQL 오류 처리 | 60분 | 어려움 | **🔴 매우 높음** |
| Graceful shutdown | 30분 | 중간 | 높음 |
| 테스트 & 버그 수정 | 90분 | 중간 | 높음 |
| **합계** | **~425분 (7시간)** | | |

**예상**: 실제로는 **8~10시간** 소요 가능성
- 버그 수정: +1~2시간
- 예기치 않은 문제: +1시간
- 테스트 반복: +30분

**평가**: ⚠️ **가능하지만 여유가 없음 (예비 시간 30분 이하)**

---

## 4. 아키텍처 현실성

### 현황: ✅ **기본 구조는 건전하나, 세부 설계에서 문제**

### 4.1 서버 구조

```
accept loop (main thread)
  ├─ socket bind/listen ✅
  ├─ accept client fd ✅
  └─ job queue에 fd 추가 ⚠️ (queue full 처리?)

worker threads (4개)
  ├─ job queue에서 fd 꺼냄 ✅
  ├─ HTTP 요청 읽기 🔴 (부분 수신 처리?)
  ├─ SQL 실행 (DB mutex 보호) ✅
  ├─ JSON 응답 생성 🔴 (메모리는? 버퍼는?)
  └─ socket write + close ✅
```

### 4.2 구체적 리스크

#### 🔴 **리스크 1: HTTP 파싱 - 부분 수신 처리 없음**

계획 문제점:
```markdown
03-api-contract.md:
"POST /query HTTP/1.1
Content-Type: text/plain
Content-Length: 1024

SELECT * FROM users;"
```

현실의 문제:
```c
// 다음과 같은 상황을 처리하지 않음:
// 1. recv()가 512 바이트만 받고 반환
// 2. header의 Content-Length를 먼저 파싱한 후 body를 그만큼 읽어야 함
// 3. \r\n vs \n 문제 (Windows vs Unix)
// 4. header 없이 body만 오는 경우 (telnet 테스트)

char buffer[4096];
int n = recv(client_fd, buffer, 4096, 0);
// n < content_length인 경우를 어떻게 처리?
// 다시 recv()? 버퍼 위치는? 타임아웃은?
```

계획에서 언급 부분:
- 06-implementation-order.md Step 3: "header를 빈 줄까지 읽습니다" ← 단순화된 표현
- 실제로는: `fgets()` 또는 반복 `recv()` + 파싱 필요

**평가**: 🔴 **치명적 누락**

#### 🔴 **리스크 2: SQL 오류 메시지 포맷 불명확**

계획 문제점:
```markdown
05-db-engine-integration.md:
"SQL_STATUS_SYNTAX_ERROR: ok: false와 SQL 오류 메시지 반환"
```

현실의 문제:
```
기존 sql_processor가 반환하는 오류 포맷은?
- enum error code인가? (1064, 1046 등)
- 문자열 메시지인가?
- SQL state는 어디서 가져오나? (42000, 40001 등)

계획: 03-api-contract.md에서 정의
{
  "ok": false,
  "status": "syntax_error",
  "error_code": 1064,
  "sql_state": "42000",
  "message": "ERROR 1064 (42000): ..."
}

하지만 기존 sql_execute()가 이 정보를 어떤 형식으로 반환하는지
05 문서에도 명시되지 않음.

문제:
- SQLResult 구조에 이런 필드가 있는가?
- 없다면 어디서 가져오는가?
- SQL 오류 메시지 문자열을 JSON에서 escape해야 하는가?
  (예: "error: \"quoted value\"" ← 쿼트 처리)
```

**평가**: 🔴 **치명적 누락** (인터페이스 불일치)

#### 🟠 **리스크 3: JSON 직렬화 메모리 관리 미결정**

계획에서:
```markdown
04-thread-pool-and-concurrency.md:
"sqlite 결과를 JSON 문자열로 변환: DB lock 필요.
client socket에 응답 쓰기: DB lock 불필요.
단, 응답 문자열을 먼저 완성한 뒤 lock을 해제합니다."
```

현실의 문제:
```c
// 방법 1: 고정 크기 버퍼
char response[4096];
int len = sprintf(response, "{\"ok\": true, \"rows\": [...]");
// 문제: SELECT 결과가 4096을 초과하면 버퍼 오버플로우

// 방법 2: 동적 할당
char *response = malloc(initial_size);
// 응답 길이를 모르므로 realloc 반복 필요? 또는 초기값을 크게?
// 메모리 누수 위험: 모든 error path에서 free() 필요

// 방법 3: 스트림 직접 쓰기 (socket write while generating JSON)
// 문제: DB lock 범위가 길어짐 (socket write까지 포함)
//       다른 worker가 오래 대기

계획은 "응답 문자열을 먼저 완성한 뒤"라고 했는데,
어느 메모리에 완성하는지, 오버플로우를 어떻게 방지하는지 명시 없음.
```

**평가**: 🟠 **높은 리스크** (구현 방식 미정)

#### 🟠 **리스크 4: Worker Thread Shutdown 메커니즘 없음**

계획에서:
```markdown
04-thread-pool-and-concurrency.md:
"worker는 서버가 종료될 때까지 반복 실행합니다.
queue가 비어 있으면 pthread_cond_wait()로 대기합니다.
새 job이 들어오면 pthread_cond_signal() 또는
pthread_cond_broadcast()로 worker를 깨웁니다."
```

현실의 문제:
```c
// Worker 루프
while (1) {
    pthread_mutex_lock(&queue_mutex);
    while (queue->count == 0) {
        pthread_cond_wait(&queue_cond, &queue_mutex);
    }
    job = dequeue();
    pthread_mutex_unlock(&queue_mutex);

    process_job(job);
}

// 서버 종료 시: "SIGTERM을 받으면 어떻게 하는가?"
// 1. 모든 worker가 cond_wait 상태에서 대기 중
// 2. main thread가 SIGTERM을 받음
// 3. main은 worker를 어떻게 깨우는가?
//    - 가짜 job을 queue에 넣고 broadcast? (hack)
//    - 다른 신호? (SIGUSR1?)
//    - 아무것도 안 하고 exit()? (fd leak, thread kill)

계획: 06-implementation-order.md Step 6에서
"작업 큐가 가득 차면 503 Service Unavailable을 반환합니다"라고만 했음.
Shutdown 전략이 명시되지 않음.
```

**평가**: 🟠 **높은 리스크** (Graceful shutdown 불가능)

#### 🟠 **리스크 5: Queue Full 처리의 구현 복잡성**

계획에서:
```markdown
02-server-architecture.md:
"queue가 가득 찬 경우 accept loop는 503 응답 후 연결을 닫거나,
여유가 생길 때까지 잠시 대기합니다. MVP는 503 응답을 기본값으로 둡니다."
```

현실의 문제:
```c
// accept loop
while (1) {
    client_fd = accept(listen_fd, ...);

    if (!can_enqueue(queue)) {
        // 503 응답을 생성하고 보내야 함
        // 그런데 이 부분도 HTTP 응답 생성 코드가 필요
        const char *response_503 =
            "HTTP/1.1 503 Service Unavailable\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 21\r\n"
            "Connection: close\r\n"
            "\r\n"
            "Server too busy\r\n";

        write(client_fd, response_503, strlen(response_503));
        close(client_fd);
    } else {
        enqueue(queue, client_fd);
    }
}

// 문제:
// - 503 응답 코드가 따로 필요함
// - 일반 응답 생성 코드와 별도로 관리
// - 복사-붙여넣기 오류 위험
```

**평가**: 🟠 **중간 리스크** (구현 번거로움)

---

## 5. 동시성 / 안정성 리스크

### 현황: ⚠️ **고수준 전략은 좋으나, 구현 세부에서 위험**

### 5.1 Thread Safety 분석

#### ✅ **안전한 부분**

```c
// Queue는 mutex로 보호됨
pthread_mutex_lock(&queue_mutex);
enqueue(job);
pthread_cond_signal(&queue_cond);
pthread_mutex_unlock(&queue_mutex);

// DB access는 전역 mutex로 보호됨
pthread_mutex_lock(&db_mutex);
result = sql_execute(shared_table, sql);
serialize_to_json(result, buffer);
pthread_mutex_unlock(&db_mutex);
```

**평가**: ✅ Coarse-grained lock이지만 현실적

#### ⚠️ **주의 필요한 부분**

**문제 1: SQLResult 포인터 유효성**

```c
pthread_mutex_lock(&db_mutex);
SQLResult result = sql_execute(shared_table, sql);

// result.records는 Record * 배열인데,
// 각 Record는 shared_table이 소유함
// 예:
// result.records[0]->id
// result.records[0]->name

// 아직 lock 상태이므로 OK
json_buf = malloc(4096);
for (int i = 0; i < result.row_count; i++) {
    // records[i]를 접근 - 안전
    sprintf(json_buf + len, ...);
}
pthread_mutex_unlock(&db_mutex);

// ⚠️ 이제 lock 없이 socket write
write(client_fd, json_buf, strlen(json_buf));

// 문제:
// - 다른 worker가 table을 변경해도 json_buf는 이미 완성되었으므로 OK
// - 하지만 Table 포인터를 혹시 serialize 코드에서 접근한다면? (아니겠지만)
```

**평가**: ✅ 설계는 안전

**문제 2: Worker shutdown 시 deadlock 위험**

```c
// Worker 1이 DB lock 잡고 있는데,
// Main thread가 SIGTERM 받고 모든 worker를 kill()하면?
// - Lock이 해제되지 않음
// - Process exit할 때 undefined behavior
```

**평가**: 🔴 **위험** (shutdown 전략 없음)

#### 🔴 **Race Condition 위험**

**문제: INSERT 후 SELECT에서 id 불일치 가능**

```
테스트 시나리오:
seq 1 20 | xargs -n1 -P8 -I{} curl ... "INSERT INTO users VALUES ('User{}', {});"

실제 동작:
- curl 1: INSERT User1, age=1 → 반환 id=1
- curl 2: INSERT User2, age=2 → 반환 id=2
- ...
- curl 8: INSERT User8, age=8 → 반환 id=8
- (동시에 8개가 실행 중)

문제:
- 각 worker가 db_mutex를 획득했을 때 table의 상태가 다름
- id auto-increment를 sql_execute()가 어떻게 하는지 불명확
- 혹시 table->id_counter를 비동기적으로 접근하면 id 중복 가능

계획에서 04-thread-pool.md에서:
"기존 Table, BPTree, Record 구조는 thread-safe하지 않으므로
DB mutex로 보호합니다"

그런데 sql_execute() 함수가 진짜 thread-safe한지 확인했는가?
id generation이 정확한가?
```

**평가**: ⚠️ **중간 리스크** (기존 코드 신뢰도)

### 5.2 Deadlock 분석

```
Lock 순서: queue_mutex → (signal) → db_mutex → socket write

일반적인 경우:
- worker는 queue_mutex를 잠깐만 잡음
- db_mutex를 잠깐만 잡음
- 다른 lock이 없음

Deadlock 가능성: 낮음 ✅

하지만:
- 만약 queue enqueue 중에 memory allocation fail하면?
- Worker가 db_mutex를 잡은 상태에서 client socket read timeout?
  (client가 응답을 못 받고 대기 중)
- Socket write가 blocking되면?
```

**평가**: ✅ 기본적으로 안전하지만, timeout 처리 없음

---

## 6. 테스트 / 발표 가능성

### 현황: ⚠️ **기본 테스트는 가능하지만, 고급 시나리오는 불안정**

### 6.1 기본 테스트 (curl 단일 요청)

```bash
curl -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "INSERT INTO users VALUES ('Alice', 20);"
```

**평가**: ✅ 대부분 성공할 가능성 높음

**예상 성공률**: 95%

### 6.2 SELECT 테스트

```bash
curl -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "SELECT * FROM users;"
```

**평가**: ✅ INSERT 이후 데이터가 있으면 성공

**예상 성공률**: 90%
(JSON 포맷 오류 가능성: 5%, 버퍼 오버플로우: 5%)

### 6.3 동시성 테스트 (높은 리스크)

```bash
seq 1 20 | xargs -n1 -P8 -I{} curl -s -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "INSERT INTO users VALUES ('User{}', {});"

# 이후
curl ... "SELECT * FROM users;"
```

**문제점 1: ID 충돌**
```
20개를 빠르게 insert하면,
id가 1~20이 아니라 중복되거나 건너뛸 수 있음.

sql_execute()가 auto-increment를 정확히 하는지 확인 필요.

테스트 검증:
- "SELECT COUNT(*) FROM users WHERE id BETWEEN 1 AND 20"
- 이게 20을 반환해야 함
- 만약 15-18이 반환되면? → 테스트 실패
```

**문제점 2: 응답 순서 보장 불가**
```
8개의 curl이 동시에 실행되면,
응답 순서가 보장되지 않음.

{"ok": true, "inserted_id": 5}  ← 2번째 curl
{"ok": true, "inserted_id": 2}  ← 1번째 curl

클라이언트 입장에서는 이상하지만, 서버는 정상.
테스트 스크립트가 이를 감지하면 혼동 가능.
```

**문제점 3: Worker pool 크기 vs 동시 요청 수**
```
계획: worker 4개, queue 64개

테스트: 8개 병렬 curl

동작:
- curl 1~4: 각 worker가 처리
- curl 5~8: queue에 대기
- curl 1이 끝나면 curl 5가 시작

총 시간: ~4배 증가 (sequential에 가까움)

이게 "병렬 처리"를 제대로 보여주는가?
아니면 단순히 "queue가 있다"는 것만 보여주는가?

발표:
"우리는 queue에 최대 64개까지 버퍼링하므로
대량 요청도 처리할 수 있습니다"
← 이 설명이 맞는가? 아니면 과장인가?
```

**평가**: 🟡 **중간 리스크** (테스트 통과는 하겠지만, 검증 불충분)

**예상 성공률**: 80%
(id 충돌: 10%, 응답 timeout: 5%, 기타: 5%)

### 6.4 발표 검증 항목

```markdown
발표 계획 (08 문서):
1. 어떤 API 서버를 만들었는가. ✅
2. 기존 SQL 처리기와 B+Tree를 어떻게 재사용했는가. ✅
3. 스레드 풀은 어떤 흐름으로 요청을 처리하는가. ✅
4. 테스트로 무엇을 검증했는가. ⚠️ (동시성 검증 약함)
5. 다른 팀과 비교해 어떤 구현 포인트가 있는가. ❌ (미정)
```

**평가**:
- 1~3번: 발표 가능 ✅
- 4번: "curl로 여러 개를 동시에 보냈다"는 설명만 가능 (깊이 부족)
- 5번: "우리는 graceful shutdown을 추가했다" 등의 차별점이 필요 (미계획)

---

## 7. 상세 구현 체크리스트 (어디서 막힐까?)

### 7.1 실제 구현 중에 만날 문제들

#### Step 1: 기본 socket + accept loop (30분)

```c
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
    int port = atoi(argv[1]);
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(listen_fd, 8);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len);

        // client_fd를 처리
        close(client_fd);
    }
}
```

**문제**: socket option 설정 (SO_REUSEADDR, TCP_NODELAY)? → 계획에 없음
**리스크**: ✅ 낮음 (기본만 해도 동작)

---

#### Step 2: HTTP 요청 파싱 (60분) 🔴 **여기서 시간 초과 가능**

```c
// 예상 코드:
char request_line[256];
char *p = buffer;
int n;

// 1. request line 읽기
while ((n = recv(client_fd, p, 1, 0)) > 0) {
    if (*p == '\n') {
        *p = '\0';
        break;
    }
    p++;
}

// 2. request line 파싱
char method[16], path[256], version[16];
sscanf(request_line, "%s %s %s", method, path, version);

// 3. header 읽기
char header_line[256];
while (1) {
    fgets(header_line, sizeof(header_line), fp);
    if (strcmp(header_line, "\r\n") == 0 ||
        strcmp(header_line, "\n") == 0) {
        break; // 빈 줄
    }
    // Content-Length 파싱
    if (strncmp(header_line, "Content-Length:", 15) == 0) {
        content_length = atoi(header_line + 15);
    }
}

// ⚠️ 문제들:
// - fgets()를 사용해야 하는데, socket fd를 fdopen()해야 함
// - fdopen() 후 fclose()하면 socket도 닫혀짐 (위험)
// - 아니면 수동 파싱: recv() 1바이트씩? (느림)
// - 또는 recv() 전체 + 포인터로 파싱? (복잡)
// - \r\n vs \n 문제 (Windows vs Unix)
```

**리스크**: 🔴 **치명적** (파싱 로직이 복잡하고 버그 나기 쉬움)

**예상 시간**: 45분 → 90분+ (디버깅 포함)

---

#### Step 3: SQL 실행 및 JSON 응답 (90분) 🔴 **가장 복잡한 부분**

```c
SQLResult result = sql_execute(shared_table, body);

// ✅ 간단한 부분: INSERT 응답
if (result.status == SQL_STATUS_OK && result.action == SQL_ACTION_INSERT) {
    char response[512];
    sprintf(response,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: 50\r\n"
        "Connection: close\r\n"
        "\r\n"
        "{\"ok\":true,\"action\":\"insert\",\"inserted_id\":%d,\"row_count\":1}",
        result.inserted_id);
    write(client_fd, response, strlen(response));
}

// ❌ 복잡한 부분: SELECT 응답 (배열)
else if (result.action == SQL_ACTION_SELECT_ROWS) {
    // 1. JSON 배열 사이즈 계산
    int size = 100;  // 초기값?
    for (int i = 0; i < result.row_count; i++) {
        // 각 row의 size를 계산
        Record *r = result.records[i];
        size += strlen(r->name) + 50;  // rough estimate
    }

    // 2. 동적 할당 or 고정 크기?
    char *buffer = malloc(size);
    if (!buffer) {
        // 메모리 부족 → 500 error
    }

    // 3. JSON 생성
    strcpy(buffer, "{\"ok\":true,\"rows\":[");
    for (int i = 0; i < result.row_count; i++) {
        Record *r = result.records[i];
        sprintf(buffer + strlen(buffer),
            "{\"id\":%d,\"name\":\"%s\",\"age\":%d}%s",
            r->id, r->name, r->age,
            (i < result.row_count - 1) ? "," : "");
    }
    strcat(buffer, "]}");

    // ⚠️ 문제들:
    // - sprintf() 오버플로우 (계산 실수 시)
    // - JSON 내 특수 문자 처리 (name에 " 또는 \ 있으면?)
    //   예: name = "O\"Brien" → JSON = {"name":"O\"Brien"} (escape 필요)
    // - 사이즈 계산의 정확성
    // - Content-Length 계산 후 HTTP header 작성

    int content_length = strlen(buffer);
    char response_header[256];
    sprintf(response_header,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n",
        content_length);

    write(client_fd, response_header, strlen(response_header));
    write(client_fd, buffer, content_length);

    free(buffer);  // ← 모든 error path에서 호출되어야 함
}
```

**리스크**: 🔴 **매우 높음** (JSON 특수 문자, 메모리 관리, 버퍼 오버플로우)

**예상 시간**: 60분 → 120분+ (버그 수정, 특수 문자 처리)

---

#### Step 4: Thread pool + mutex (90분)

```c
typedef struct {
    int client_fd;
} Job;

typedef struct {
    Job jobs[64];
    int head, tail, count;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} JobQueue;

void *worker_thread(void *arg) {
    JobQueue *queue = (JobQueue *)arg;

    while (1) {
        pthread_mutex_lock(&queue->mutex);

        while (queue->count == 0) {
            pthread_cond_wait(&queue->cond, &queue->mutex);
        }

        Job job = queue->jobs[queue->head];
        queue->head = (queue->head + 1) % 64;
        queue->count--;

        pthread_mutex_unlock(&queue->mutex);

        // Process job
        handle_client(job.client_fd);
        close(job.client_fd);
    }

    return NULL;
}

// ⚠️ 문제:
// - Worker 4개를 어떻게 kill할 것인가?
// - SIGTERM 신호를 받으면?
// - pthread_join()을 하려면 worker를 먼저 깨워야 함
// - 현재 코드는 무한 loop이고, 깨우는 방법이 없음
```

**리스크**: 🟠 **높음** (Shutdown이 구현되지 않음)

**예상 시간**: 60분 → 90분 (shutdown 처리 추가 시)

---

#### Step 5: DB mutex (30분)

```c
pthread_mutex_t db_mutex = PTHREAD_MUTEX_INITIALIZER;

void handle_client(int client_fd) {
    char body[4096];
    // ... HTTP 파싱해서 body 채우기 ...

    // DB access
    pthread_mutex_lock(&db_mutex);

    SQLResult result = sql_execute(shared_table, body);

    // JSON 생성 (여전히 lock 상태)
    char response[4096];
    format_response(response, &result);

    pthread_mutex_unlock(&db_mutex);

    // Socket write (lock 없음)
    write(client_fd, response, strlen(response));
}
```

**리스크**: ✅ 낮음 (기본 mutex 사용법)

---

### 7.2 실제 문제 발생 시나리오

#### 시나리오 1: "curl 테스트는 되는데 동시 요청 시 서버 죽음"

```
원인 분석:
1. HTTP 파싱 중 buffer overflow
   → 특정 길이의 요청이 들어오면 segfault
2. JSON 생성 중 버퍼 오버플로우
   → SELECT 결과가 4096 이상이면 crash
3. Worker shutdown 불가
   → 프로세스가 zombie가 됨

이 모든 것이 현재 계획에서 "시간이 부족하면 생략"되기 쉬운 부분들
```

#### 시나리오 2: "특정 SQL은 작동하는데 특정 SQL은 안 됨"

```
원인 분석:
1. SQL 오류 메시지 처리 미흡
   → 오류가 발생하면 응답이 이상한 JSON
   → 클라이언트가 파싱 불가
2. 기존 sql_execute()의 동작을 모르는 상태에서 JSON 변환
   → SQLResult 구조의 필드를 잘못 해석
```

#### 시나리오 3: "README대로 따라 하면 안 된다"

```
원인 분석:
1. HTTP 파싱이 미완성
   → telnet이나 특정 클라이언트에서만 동작 불가
2. Content-Length 계산 오류
   → 응답이 끝나지 않는 것처럼 보임
3. JSON 포맷 오류
   → curl로 받은 응답이 깨짐
```

---

## 8. 최종 권고사항

### 8.1 🔴 **즉시 수정 필요** (치명적 리스크)

#### 1. HTTP 파싱 명세 추가

**현재**: 06-implementation-order.md에서 "header를 빈 줄까지 읽습니다"

**필요**: 명확한 구현 예시 추가

```markdown
### HTTP 요청 파싱 상세 가이드

1. 첫 번째 라인이 "POST /query HTTP/1.1"인지 확인
   - method가 POST가 아니면 405 Method Not Allowed
   - path가 /query가 아니면 404 Not Found

2. header 읽기
   - recv()를 반복하면서 \r\n 또는 \n을 찾기
   - Content-Length: XXX에서 XXX 파싱
   - \r\n\r\n (또는 \n\n)까지 읽으면 header 완료

3. body 읽기
   - Content-Length만큼 정확히 읽기
   - 부분 수신 시 반복 recv()
   - body 끝에 null terminator 추가

예시 코드:
(C code snippet)
```

#### 2. SQL 오류 처리 명세 추가

**현재**: 05-db-engine-integration.md에서 "SQL_STATUS_SYNTAX_ERROR: ok: false와 SQL 오류 메시지 반환"

**필요**: 기존 sql_processor의 오류 포맷 확인 및 명시

```markdown
### 기존 SQL 엔진 오류 정보 추출

sql_execute() 반환 후:

- result.status: SQL_STATUS_SYNTAX_ERROR / SQL_STATUS_QUERY_ERROR
- result.error_message: (존재하는가?)
- result.error_code: (존재하는가?)

만약 위 필드가 없다면:
- SQL 엔진의 소스 코드를 확인하여 오류 정보를 추출하는 방법 정의
- 또는 JSON 응답에서 오류 부분을 단순화
  {"ok": false, "message": "SQL execution failed"}
```

#### 3. JSON 메모리 관리 전략 명시

**현재**: "응답 문자열을 먼저 완성한 뒤 lock을 해제합니다" (추상적)

**필요**: 구체적인 구현 방식 선택 및 명시

```markdown
### JSON 응답 메모리 관리

선택지 1: 고정 크기 버퍼 (단순하지만 위험)
- char response[8192]; 사용
- SELECT 결과가 8K 초과하면 오류 반환
- 장점: 구현 단순
- 단점: 큰 데이터 처리 불가

선택지 2: 동적 할당 (안전하지만 복잡)
- 추정 크기로 malloc()
- realloc() 또는 오버플로우 시 처리
- 장점: 크기 제한 없음
- 단점: 메모리 관리 복잡

MVP 권장: 선택지 1 (8192 byte 고정)
- 대부분의 테스트 케이스 커버
- 실패 시 간단한 오류 메시지 반환
```

### 8.2 🟠 **높은 우선순위 개선** (높은 리스크)

#### 1. Graceful Shutdown 메커니즘

```markdown
### Worker Shutdown 절차

방법: Sentinel job 사용

Job 구조:
typedef struct {
    int client_fd;      // -1이면 shutdown signal
} Job;

Shutdown 처리:
- Main이 SIGTERM 수신
- 모든 worker 수만큼 sentinel job 큐에 추가 (client_fd = -1)
- broadcast로 모든 worker 깨우기
- 각 worker가 sentinel job 받으면 루프 종료
- Main이 pthread_join()으로 모든 worker 대기
- Graceful shutdown 완료

코드:
for (int i = 0; i < NUM_WORKERS; i++) {
    Job sentinel = {.client_fd = -1};
    enqueue(queue, sentinel);
}
pthread_cond_broadcast(&queue->cond);

// Worker:
if (job.client_fd == -1) {
    break;  // 루프 종료
}
```

#### 2. 동시성 테스트 검증 개선

```markdown
### 동시 요청 테스트 검증

현재 시나리오:
seq 1 20 | xargs -n1 -P8 -I{} curl ... "INSERT INTO users VALUES ('User{}', {});"

문제: id 충돌 또는 건너뜀 감지 못함

개선: 검증 스크립트 추가

#!/bin/bash
# 서버 시작 (기존 데이터 제거)

# INSERT 20개
seq 1 20 | xargs -n1 -P8 -I{} curl -s ... "INSERT INTO users VALUES ('User{}', {});" > /dev/null

# 검증
RESULT=$(curl -s -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "SELECT COUNT(*) FROM users;")

# JSON에서 count 추출
COUNT=$(echo $RESULT | jq '.rows[0].count')

if [ "$COUNT" = "20" ]; then
    echo "✓ 동시성 테스트 통과"
else
    echo "✗ 동시성 테스트 실패: expected 20, got $COUNT"
fi
```

### 8.3 🟡 **낮은 우선순위** (낮은 리스크이지만 언급)

#### 1. 차별화 포인트 추가

```markdown
(08-demo-and-readme-plan.md에서 구체화)

초안: "우리는 graceful shutdown을 구현했습니다"
개선: 구체적인 구현 방식 설명

"우리의 특장점:
- Worker pool로 thread 생성 오버헤드 제거 (고정 4개)
- Bounded queue로 메모리 사용량 제한 (최대 64개)
- DB mutex로 기존 비동기 코드를 안전하게 재사용
- Graceful shutdown으로 인-플라이트 요청 안전 처리"
```

#### 2. 성능 테스트 추가 (선택사항)

```markdown
(07-test-and-quality-plan.md에 추가)

간단한 benchmark:
- 100개 INSERT의 총 시간 측정
- 순차 vs 병렬 성능 비교
- Thread pool 크기 변화에 따른 성능 변화

예:
$ time seq 1 100 | xargs -n1 -P1 -I{} curl ... "INSERT ..." (순차)
$ time seq 1 100 | xargs -n1 -P8 -I{} curl ... "INSERT ..." (병렬)
```

---

## 9. 최종 체크리스트

| 항목 | 상태 | 조치 |
|------|------|------|
| 과제 요구사항 명확화 | ✅ | - |
| HTTP 파싱 상세 명세 | ❌ | **추가 필요** |
| SQL 오류 처리 명세 | ❌ | **추가 필요** |
| JSON 메모리 관리 전략 | ❌ | **선택 필요** |
| Graceful shutdown | ❌ | **추가 필요** |
| 동시성 테스트 검증 | ⚠️ | 스크립트 개선 필요 |
| 차별화 포인트 | ❌ | 구체화 필요 |
| 예상 구현 시간 | ✅ | 7시간 (여유 < 1시간) |

---

## 10. 결론

### ✅ **강점**
1. 요구사항을 모두 포함
2. 기본 아키텍처가 합리적
3. 계층 분리가 명확
4. 테스트 계획이 체계적
5. 발표 계획이 구체적

### ❌ **약점**
1. HTTP 파싱 구현 난이도 과소평가
2. SQL 오류 처리 스펙 불명확
3. JSON 메모리 관리 전략 미정
4. Graceful shutdown 메커니즘 없음
5. 실제 구현 예시 부재

### 🎯 **권고**
1. **필수**: 위의 8.1 "즉시 수정 필요" 3가지 항목 추가
2. **강력 권고**: 8.2의 graceful shutdown 메커니즘 추가
3. **개선 바람**: 실제 C 코드 스켈레톤 예시 추가
4. **현실적 기대**: 하루 구현으로 "완벽한" 코드보다 "동작하는" 코드 우선

---

**최종 평가**: **70점 → 85점으로 올릴 수 있음** (위 권고사항 반영 시)

**현재 계획의 위험도**: 🟠 **중간-높음** (실행 가능하지만 예상 외 문제 가능성 높음)
