# 04. Thread Pool and Concurrency

## 코드 주석 원칙

스레드 풀과 동시성 코드를 작성할 때는 mutex, condition variable, queue 상태 변경, DB lock 획득과 해제 지점에 한국어 주석을 반드시 남깁니다. 주석은 단순 동작 설명보다 race condition을 막기 위한 이유와 lock을 잡는 범위를 설명하는 데 집중합니다.

이 문서는 스레드 풀 구성, 작업 큐, 공유 DB 엔진 보호 전략을 정의합니다.

## 목표

요청마다 새 스레드를 만들지 않고, 서버 시작 시 미리 만든 worker thread들이 SQL 요청을 처리하도록 만듭니다. 이 구조는 동시 요청 처리 능력을 보여주면서도 구현과 설명이 단순합니다.

## 기본 구조

```text
main thread
  -> listen socket
  -> accept client fd
  -> enqueue job
  -> signal worker

worker thread
  -> wait for job
  -> dequeue client fd
  -> read HTTP request
  -> execute SQL with DB mutex
  -> write HTTP response
  -> close client fd
```

## Thread Pool 기본값

- worker 개수: 4개.
- queue 크기: 64개.
- worker는 서버가 종료될 때까지 반복 실행합니다.
- queue가 비어 있으면 `pthread_cond_wait()`로 대기합니다.
- 새 job이 들어오면 `pthread_cond_signal()` 또는 `pthread_cond_broadcast()`로 worker를 깨웁니다.

## Job Queue 정책

- job에는 최소한 client socket fd를 저장합니다.
- queue는 circular buffer로 구현합니다.
- `head`, `tail`, `count`, `capacity`를 유지합니다.
- enqueue/dequeue는 queue mutex 안에서만 수행합니다.
- queue가 가득 차면 MVP에서는 해당 연결에 `503 Service Unavailable`을 보내고 닫습니다.

## DB 동시성 정책

기존 `Table`, `BPTree`, `Record` 구조는 thread-safe하게 설계되어 있지 않습니다. 따라서 SQL 실행 구간 전체를 하나의 DB mutex로 보호합니다.

```text
pthread_mutex_lock(&db_mutex)
result = sql_execute(shared_table, sql)
serialize result while records are still valid
sql_result_destroy(&result)
pthread_mutex_unlock(&db_mutex)
```

중요한 점은 `SQLResult.records`가 `Record *` 포인터 배열이라는 것입니다. 응답 body를 만들기 전에 다른 thread가 table을 변경하면 위험할 수 있으므로, MVP에서는 SQL 실행과 응답 직렬화를 같은 DB lock 안에서 끝냅니다.

## 잠금 범위

- HTTP 요청 읽기: DB lock 불필요.
- SQL 문자열 검증: DB lock 불필요.
- `sql_execute()` 호출: DB lock 필요.
- `SQLResult`를 JSON 문자열로 변환: DB lock 필요.
- client socket에 응답 쓰기: DB lock 불필요. 단, 응답 문자열을 먼저 완성한 뒤 lock을 해제합니다.

## 확장 후보

MVP 이후에는 `pthread_rwlock_t`를 사용해 SELECT는 read lock, INSERT는 write lock으로 분리할 수 있습니다. 단, 현재 목표는 완성도 높은 MVP와 설명 가능한 동시성 구조이므로 전역 DB mutex를 기본값으로 둡니다.

## 발표 포인트

- 스레드 풀은 요청 폭주 시 스레드 생성 비용을 줄입니다.
- bounded queue는 서버가 감당 가능한 작업량을 명확히 제한합니다.
- DB mutex는 기존 비동기 안전하지 않은 자료구조를 안전하게 재사용하기 위한 선택입니다.
- MVP에서는 성능 최적화보다 데이터 일관성과 구현 안정성을 우선합니다.
