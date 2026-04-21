# Request Flow And Module Boundaries

## 전체 구조

```text
client
-> TCP socket
-> main accept loop
-> job queue
-> worker thread
-> HTTP parse
-> route
-> SQL mapping
-> db adapter
-> TEAM7 engine
-> JSON response
-> close(fd)
```

## 모듈 경계

### 1. `main`

- DB 초기화
- thread pool 초기화
- listen socket 생성
- `accept` loop 수행
- 새 `client fd`를 job queue에 넣음

메인 스레드는 요청 파싱이나 SQL 실행을 하지 않는다.

### 2. `thread_pool`

- 고정 크기 worker 생성
- 큐에서 `client fd`를 꺼냄
- worker가 요청 처리 전체를 수행

### 3. `server/http`

- request read
- request line parse
- 최소 header 처리
- `Content-Length` 기반 body read
- route 분기
- JSON 응답 작성

### 4. `db_adapter`

- 서버가 TEAM7 엔진을 직접 호출하지 않도록 중간 인터페이스 제공
- `db_init`, `db_execute_sql`, `db_shutdown` 제공
- `SQLResult`를 서버용 결과 구조로 변환
- DB 락을 여기서 관리

### 5. `team7_engine`

- `sql_execute`
- `table_create`
- `table_destroy`
- B+Tree 기반 `id` 조회

## 동시성 전략

### 작업 큐

- producer-consumer 구조
- producer: 메인 스레드
- consumer: worker threads
- `mutex + condition variable` 사용

### DB 락

- 전역 DB 인스턴스 1개
- read-write lock 1개
- `SELECT`: read lock
- `INSERT`: write lock

### 병렬 처리 설명

- 여러 연결은 thread pool에 의해 동시에 처리될 수 있다.
- 읽기 요청은 동시에 DB 접근이 가능하다.
- 쓰기 요청은 DB 무결성을 위해 직렬화된다.

## HTTP 지원 범위

- 요청당 연결 1회
- `Connection: close`
- `GET`, `POST`만 지원
- `Content-Length` 있는 body만 지원

지원하지 않는 기능:

- keep-alive
- chunked encoding
- pipelining
- 범용 header 처리

## 메모리 소유권 원칙

- request buffer는 worker가 만들고 worker가 해제한다.
- `DbResult` 내부 동적 메모리는 응답 작성 후 `db_result_destroy()`로 정리한다.
- TEAM7 `SQLResult`는 `db_adapter` 안에서만 다루고, 외부로 노출하지 않는다.

## 구현 우선순위 연결

1. TEAM7 엔진 래핑 가능성 확인
2. DB 어댑터 구현
3. `GET /health` 서버 구현
4. thread pool + job queue 연결
5. 최소 HTTP 파싱
6. `POST /users`, `GET /users`, `GET /users/{id}` 연결
7. 락 적용 및 동시성 확인
