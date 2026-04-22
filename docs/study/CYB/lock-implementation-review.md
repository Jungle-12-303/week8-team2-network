# Lock 구현 점검 피드백 (CYB)

> 기준: `cyb` 브랜치 기준으로 `server/api.c`, `server/server.c`, `server/thread_pool.c`, `sql_processor/sql.c` 집중 점검  
> 작성일: 2026-04-22

---

## 요약

| 영역 | 상태 | 비고 |
|------|------|------|
| mutex → rwlock 전환 | ✅ 완료 | SELECT 동시 처리 가능 |
| lock 범위 (JSON 직렬화 분리) | ✅ 완료 | mvp-review Issue 1 해결됨 |
| thread_pool mutex/cond 패턴 | ✅ 적절 | critical section 최소화 |
| study guide 문서 동기화 | ❌ 불일치 | 코드 변경이 문서에 반영 안 됨 |
| unlock 후 records 포인터 사용 | ⚠️ 잠재적 위험 | 현재는 안전, 가정이 명시되지 않음 |
| SQL_LOCK_NONE 암묵적 가정 | ⚠️ 미명시 | table 접근 안 하는 경로라 안전하지만 코드에 표현 없음 |
| SQL double-parsing | ⚠️ 경미 | lock 결정용 + 실행용으로 두 번 파싱 |

---

## 1. 잘 된 것들

### 1-1. pthread_rwlock_t 전환 완료

**코드 위치**: `server/server.c:22`, `server/api.c:108~143`

```c
// server.c:22
pthread_rwlock_t db_lock;

// api.c:127~137
if (lock_mode == SQL_LOCK_READ) {
    pthread_rwlock_rdlock(db_lock);   // SELECT → 읽기 잠금
} else if (lock_mode == SQL_LOCK_WRITE) {
    pthread_rwlock_wrlock(db_lock);   // INSERT → 쓰기 잠금
}
```

- `pthread_mutex_t`에서 `pthread_rwlock_t`로 전환됨
- SELECT 요청 여러 개가 동시에 `rdlock`을 잡을 수 있어 읽기 병렬성 확보
- INSERT 1개가 `wrlock`을 잡으면 SELECT들이 대기 → 쓰기 일관성 보장

### 1-2. lock 범위가 DB 접근에만 한정됨

**코드 위치**: `server/api.c:139~179`

```c
sql_result = sql_execute(table, sql);   // DB 접근 (잠금 안)

if (locked) {
    pthread_rwlock_unlock(db_lock);     // 여기서 즉시 해제
    locked = 0;
}

// --- 잠금 밖 ---
json_buffer_init(&buffer, 256);
api_build_success_response(&sql_result, &buffer);   // JSON 직렬화
```

mvp-review의 Issue 1에서 지적했던 "JSON 직렬화까지 mutex 잡고 있는 문제"가 해결됨.  
lock을 최소 시간 동안만 보유하는 올바른 패턴.

### 1-3. thread_pool의 mutex/cond 패턴

**코드 위치**: `server/thread_pool.c:10~35`, `93~111`

```c
// 워커 (소비자)
pthread_mutex_lock(&pool->mutex);
while (!pool->stop_requested && pool->size == 0) {
    pthread_cond_wait(&pool->cond, &pool->mutex);   // 큐가 빌 때 대기
}
client_fd = pool->jobs[pool->head].client_fd;
pool->head = (pool->head + 1) % pool->queue_capacity;
pool->size--;
pthread_mutex_unlock(&pool->mutex);

// submit (생산자)
pthread_mutex_lock(&pool->mutex);
pool->jobs[pool->tail].client_fd = client_fd;
pool->tail = ...;
pool->size++;
pthread_cond_signal(&pool->cond);   // 워커 깨움
pthread_mutex_unlock(&pool->mutex);
```

- `head`, `tail`, `size`, `stop_requested` 모두 mutex 보호 아래 접근 → 레이스 컨디션 없음
- `cond_signal`을 mutex 보유 상태에서 호출 → POSIX 권고 사항 준수
- `stop_requested && size == 0` 조건: 진행 중인 작업 소진 후 종료 → graceful shutdown

---

## 2. 문제점 및 피드백

### [문제 1] study guide 문서가 실제 코드와 다르다

**심각도**: 높음 (학습 혼란 유발)

**위치**: `docs/study/CYB/project-study-guide.md` 섹션 2, 섹션 4

`project-study-guide.md`의 아키텍처 다이어그램(섹션 2)과 시퀀스 다이어그램(섹션 4)이 아직 옛날 구조를 설명하고 있다.

**문서에 쓰인 내용 (현재)**:
```
api.c:
  pthread_mutex_lock(db_mutex)
  → sql_execute(table, sql)
  → JSON 직렬화
  → pthread_mutex_unlock(db_mutex)
```

**실제 코드 (현재)**:
```
api.c:
  pthread_rwlock_rdlock 또는 pthread_rwlock_wrlock (sql 종류에 따라)
  → sql_execute(table, sql)
  → pthread_rwlock_unlock  ← 여기서 즉시 해제
  (잠금 밖에서) JSON 직렬화
```

구체적으로 수정이 필요한 부분:
- 섹션 2 다이어그램의 `api.c` 박스: `pthread_mutex_lock(db_mutex)` → `pthread_rwlock_rdlock/wrlock(db_lock)`
- 섹션 4 시퀀스 다이어그램의 `api_handle_query(table, db_mutex, sql, ...)` → `db_lock`으로 수정
- 섹션 4에서 `pthread_mutex_lock/unlock`이 JSON 직렬화를 감싸는 것처럼 표현된 부분 → unlock 이후에 JSON 빌딩이 일어남을 명시
- `mvp-review.md`의 Issue 1: 이미 해결된 문제이므로 "해결됨" 표시 추가

---

### [문제 2] unlock 후 records 포인터를 쓰는 가정이 암묵적이다

**심각도**: 중간 (현재는 안전하지만, DELETE 추가 시 바로 버그)

**위치**: `server/api.c:141~179`

lock을 해제한 뒤에 `sql_result.records[]` 포인터를 통해 `record->name`, `record->id`, `record->age`에 접근한다.

```c
pthread_rwlock_unlock(db_lock);  // unlock

// 이 시점부터 다른 스레드가 table 구조를 바꿀 수 있음
api_build_success_response(&sql_result, &buffer);  // records[] 포인터 역참조
```

현재 이것이 안전한 이유:
- `table_copy_records()`가 포인터 배열을 별도 malloc으로 복사한다 (`table.c:54~73`)
- 하지만 배열 안의 `Record*`들은 여전히 table 내부 힙을 가리킨다
- INSERT는 새 Record를 `malloc`하고 `rows[]` 배열에 추가하는 것이므로, 기존 Record 주소는 바뀌지 않는다
- DELETE/UPDATE가 없으므로 기존 Record가 free되거나 수정될 일이 없다

안전하지 않게 되는 경우:
- DELETE 구현 시: 다른 스레드가 기존 Record를 `free`하면 use-after-free
- UPDATE 구현 시: lock 밖에서 읽는 동안 다른 스레드가 값을 바꾸면 torn read

현재 상태에서도 코드 어디에도 이 안전성의 근거가 설명되지 않는다.

**개선 방안** (둘 중 하나):

A. 지금처럼 lock 밖에서 쓰되, 코드에 근거를 명시한다:
```c
/* safe because INSERT only appends new Records — existing pointers remain valid */
pthread_rwlock_unlock(db_lock);
```

B. `sql_result.records[]`가 Record 포인터가 아니라 Record 값 사본을 가지도록 바꾼다:
```c
typedef struct Record {
    int id;
    char name[64];
    int age;
} Record;  // 이미 값 타입이므로 records를 Record[]로 복사하면 완전히 독립됨
```

---

### [문제 3] SQL_LOCK_NONE 경로가 table에 접근하지 않는다는 근거가 없다

**심각도**: 낮음 (현재 동작은 맞음)

**위치**: `server/api.c:126~139`, `sql_processor/sql.c:121~140`

```c
lock_mode = sql_determine_lock_mode(sql);
// EXIT, QUIT, 알 수 없는 SQL → SQL_LOCK_NONE
// → lock을 잡지 않고 sql_execute 호출

sql_result = sql_execute(table, sql);
```

`sql_execute`에서 EXIT/QUIT는 table 접근 전에 반환하고, 알 수 없는 SQL도 파싱 실패 후 반환한다 (`sql.c:491~523`). 그러므로 lock 없이 호출해도 문제없다.

하지만 코드만 읽으면 이 사실을 바로 알기 어렵다. `sql_determine_lock_mode`가 NONE을 반환할 때 왜 lock 없이 실행해도 되는지 설명이 없다.

**개선 방안**: api.c의 NONE 분기에 한 줄 주석으로 이유를 밝힌다.
```c
/* SQL_LOCK_NONE: EXIT/QUIT or unknown — sql_execute touches no table data */
```

---

### [문제 4] SQL을 두 번 파싱한다

**심각도**: 낮음 (기능 영향 없음, 코드 냄새)

**위치**: `server/api.c:126`과 `sql.c:492`

`sql_determine_lock_mode(sql)` 호출에서 "INSERT"인지 "SELECT"인지 한 번 파싱하고, `sql_execute(table, sql)` 호출에서 또 처음부터 파싱한다.

현재 SQL 크기(최대 4KB)와 요청 빈도에서 성능 문제는 없다. 하지만 lock 결정과 실행이 다른 파싱 경로를 타므로, 나중에 `sql_determine_lock_mode`를 수정하고 `sql_execute`를 수정하지 않으면 lock 타입이 실제 실행 동작과 어긋날 수 있다.

**개선 방향 (선택)**: `sql_execute`가 `SQLLockMode`를 결과에 포함시켜 반환하게 하면 파싱을 한 번으로 줄이고, 호출자가 나중에 대조 가능해진다. 다만 인터페이스 변경이 크므로 현 MVP 단계에서는 주석으로 의존 관계를 표시하는 수준으로 충분하다.

---

## 3. 가이드라인 준수 여부 종합

| 가이드라인 항목 | 실제 코드 | 판정 |
|----------------|-----------|------|
| 뮤텍스로 DB 보호 (STEP 3) | rwlock으로 격상 | ✅ 초과 달성 |
| JSON 직렬화는 lock 밖 | lock 해제 후 직렬화 | ✅ |
| 큐 접근 시 mutex 보호 | head/tail/size 모두 보호 | ✅ |
| cond_wait 올바른 while 루프 | while(!stop && size==0) | ✅ |
| graceful shutdown (size==0 대기) | stop_requested && size==0 | ✅ |
| study guide와 코드 일치 | 문서가 뒤처짐 | ❌ |
| 잠금 범위 최소화 | DB 접근에만 한정 | ✅ |

---

## 4. 우선순위 행동 제안

1. **즉시**: `project-study-guide.md` 섹션 2, 섹션 4의 `db_mutex` → `db_lock` 표현 수정 및 JSON 직렬화 위치 수정
2. **즉시**: `mvp-review.md` Issue 1에 "해결됨" 표시 및 rwlock 전환 내용 반영
3. **선택**: `api.c`의 unlock 직후에 records 포인터 안전성 근거 한 줄 주석 추가
4. **선택**: `api.c`의 SQL_LOCK_NONE 분기에 "table 접근 없음" 근거 주석 추가

---

*작성: CYB, 2026-04-22*
