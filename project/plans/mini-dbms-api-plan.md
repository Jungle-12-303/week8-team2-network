# Team7 엔진 기반 미니 DBMS API 서버 최소 구현 플랜

## 목적

이번 주 과제 요구사항을 만족하는 수준으로, 이전 주차 TEAM7의 SQL 처리기와 B+Tree 엔진을 재사용해 외부 클라이언트가 호출 가능한 C 기반 API 서버를 만든다.

핵심 목표는 새 DB 엔진을 설계하는 것이 아니라, 기존 엔진을 서버 뒤에 안정적으로 연결하고 thread pool 기반 병렬 요청 처리를 데모 가능 상태까지 완성하는 것이다.

## 과제 요구사항 정리

- 외부 클라이언트가 API를 통해 DB 기능을 사용할 수 있어야 한다.
- Thread Pool이 있어야 한다.
- SQL 요청을 병렬 처리해야 한다.
- 기존 SQL 처리기와 B+Tree를 재사용해야 한다.
- 구현 언어는 C다.
- 테스트와 데모가 가능해야 한다.

## Summary

이 프로젝트는 TEAM7 레포 `week7-tem7-sql_processor_2`의 엔진 코드를 기반으로 하되, 서버 연동에 필요한 범위에서는 수정 가능하다는 전제로 진행한다.

재사용 대상 레포:

- `https://github.com/Jungle-12-303/week7-tem7-sql_processor_2`

최소 구현 목표는 아래 네 가지다.

1. `users` 단일 테이블 기준 REST API 제공
2. 고정 크기 thread pool + job queue 구현
3. TEAM7 엔진을 감싼 DB 어댑터 계층 구현
4. README만으로 실행, 테스트, 데모 재현 가능 상태 확보
5. 로컬과 Docker 두 경로에서 같은 서버를 실행 가능하게 구성

## 역할 기반 개발 방식

이번 프로젝트는 아래 5개 페르소나를 명시적으로 사용해 진행한다.

- PM
- Architect
- Developer
- QA
- Reviewer

기본 흐름:

```text
PM -> Architect -> Developer -> QA -> Reviewer -> 수정 반복 -> 완료
```

개발 원칙:

- 실제 구현을 시작하기 전에 `project/ai-context` 문서를 먼저 읽고 반영한다.
- 구현은 코드 작성에서 끝내지 않고, 검증 후 적절한 단위로 커밋하는 것까지 포함한다.
- 커밋은 기능, 리팩터링, 테스트, 문서 변경을 가능한 한 분리해 리뷰 가능한 크기로 유지한다.
- 커밋 메시지는 `project/ai-context/commit-convention.md`를 따른다.

역할 문서:

- [PM](/Users/choeyeongbin/week8-team2-network/project/personas/pm.md)
- [Architect](/Users/choeyeongbin/week8-team2-network/project/personas/architect.md)
- [Developer](/Users/choeyeongbin/week8-team2-network/project/personas/developer.md)
- [QA](/Users/choeyeongbin/week8-team2-network/project/personas/qa.md)
- [Reviewer](/Users/choeyeongbin/week8-team2-network/project/personas/reviewer.md)

## 이번 주 최소 구현 범위

- TEAM7 엔진 코드 분석 및 필요한 범위 수정
- 단일 프로세스 C 서버 구현
- 고정 크기 Thread Pool + 작업 큐 구현
- `users` 단일 테이블 기준 API 제공
- `GET /health`, `POST /users`, `GET /users`, `GET /users/{id}` 구현
- DB 전체 단위 read-write lock 기반 동시성 제어
- 제한된 HTTP subset 지원
- Docker 기반 빌드/실행/테스트 경로 제공
- 실행 방법, 테스트 방법, 데모 흐름이 포함된 README 작성

## 이번 주 제외 범위

- 복수 테이블 관리
- 트랜잭션
- 인증/인가
- keep-alive
- chunked encoding
- query parameter 기반 필터 조회
- fine-grained lock
- 고급 graceful shutdown
- 디스크 영속성 확장
- 범용 SQL API 공개
- 프로덕션 배포 오케스트레이션
- 쿠버네티스 배포
- 고급 이미지 최적화

## 핵심 수정 방향

### 1. TEAM7 엔진은 재사용하되, 필요하면 수정한다

- TEAM7 코드를 100% 원형 유지하는 것을 목표로 하지 않는다.
- 서버 연동, 결과 구조화, 메모리 정리, thread-safe 대응을 위해 필요한 범위의 수정은 허용한다.
- 단, 수정 목적은 새 엔진 개발이 아니라 기존 엔진의 서버 적합성 확보에 한정한다.

재사용 및 수정 대상 핵심 요소:

- `Table`
- `BPTree`
- `sql_execute(Table *table, const char *input)`
- `SQLResult`

수정 가능 예시:

- CLI 출력 의존 제거
- `SQLResult` 구조 정리
- 메모리 해제 함수 추가
- 서버 연동에 필요한 반환값 보강
- 전역 상태 정리 또는 의존성 축소

### 2. HTTP 범위를 강하게 제한한다

이 프로젝트는 완전한 HTTP/1.1 서버를 목표로 하지 않는다.

지원 범위:

- 요청당 연결 1회 처리
- `Connection: close` 방식
- `Content-Length` 기반 body 읽기
- JSON request/response
- `GET`과 `POST`만 지원

지원하지 않는 범위:

- keep-alive
- chunked encoding
- pipelining
- 압축
- 범용 header 처리

### 3. 실행 환경은 로컬과 Docker 두 경로를 같이 제공한다

- 로컬 실행 경로는 유지한다.
- Docker 환경은 팀원 공통 실행 경로로 추가한다.
- 서버 코드는 동일 바이너리를 기준으로 실행한다.
- Docker 환경은 개발 편의와 발표 재현성을 위한 수단으로만 사용한다.

### 4. 소켓 처리 책임을 단순하게 고정한다

기본 흐름:

```text
accept
-> client fd를 job queue에 enqueue
-> worker가 read/parse/execute/write/close 전체 처리
```

역할 분리:

- 메인 스레드: `accept`만 담당
- worker thread: 요청 읽기, 파싱, SQL 실행, 응답 작성, 연결 종료까지 담당

이 구조를 쓰는 이유:

- fd 수명 관리가 단순하다.
- 응답 write 책임이 명확하다.
- C에서 메인/worker 간 버퍼 소유권이 덜 꼬인다.

### 5. 동시성은 안전성 우선으로 단순화한다

- 전역 DB 인스턴스 1개를 둔다.
- DB 접근은 read-write lock 1개로 보호한다.
- `SELECT`는 read lock
- `INSERT`는 write lock

병렬 처리 정의:

- 요청 수락과 worker 분배는 병렬이다.
- 읽기 요청은 병렬 실행 가능하다.
- 쓰기 요청은 무결성을 위해 직렬화된다.

## 서버 아키텍처

### 구성 요소

- `main`: 서버 시작, DB 초기화, thread pool 생성, accept loop
- `thread_pool`: worker 생성 및 종료
- `job_queue`: `client_fd` 전달용 큐
- `http`: 최소 request parse, response write
- `router`: endpoint 분기
- `db_adapter`: TEAM7 엔진 연결
- `sql_mapper`: REST 요청을 내부 SQL로 변환

### 요청 처리 흐름

```text
client
-> socket connect
-> accept
-> client fd enqueue
-> worker picks fd
-> HTTP request read
-> parse
-> route
-> SQL mapping
-> db_execute_sql()
-> JSON response write
-> close(fd)
```

## DB 엔진 연결 방식

서버 계층에서 사용할 인터페이스는 아래 3개로 최소화한다.

- `db_init(...)`
- `db_execute_sql(const char *sql, DbResult *out)`
- `db_shutdown(...)`

원칙:

- TEAM7의 CLI 프로그램 자체를 재사용하지 않는다.
- 엔진 코드만 서버에서 호출 가능한 형태로 분리한다.
- 필요하면 TEAM7 코드를 수정해 서버 친화적으로 바꾼다.
- 결과는 stdout 출력이 아니라 구조체 기반으로 다룬다.

우선 검증할 것:

- `INSERT`
- `SELECT *`
- `SELECT ... WHERE id = ...`

## REST API 최소셋

### 1. `GET /health`

- 용도: 서버 생존 확인
- 성공 응답: `200 OK`
- 예시:

```json
{
  "ok": true
}
```

### 2. `POST /users`

- request body:

```json
{
  "name": "Alice",
  "age": 20
}
```

- 내부 변환:

```sql
INSERT INTO users VALUES ('Alice', 20);
```

- 성공 응답: `201 Created`
- 기본 응답 예시:

```json
{
  "ok": true,
  "message": "created"
}
```

주의:

- insert 후 생성 id를 안정적으로 얻기 어렵다면, 최소 구현에서는 응답에 `id`를 넣지 않는다.

### 3. `GET /users`

- 내부 변환:

```sql
SELECT * FROM users;
```

- 성공 응답: `200 OK`

### 4. `GET /users/{id}`

- 내부 변환:

```sql
SELECT * FROM users WHERE id = {id};
```

- 존재 시 `200 OK`
- 없으면 `404 Not Found`

### 실패 응답 규칙

- malformed request: `400 Bad Request`
- 잘못된 path: `404 Not Found`
- 엔진 실행 실패: `500 Internal Server Error`

모든 실패 응답은 JSON으로 통일한다.

## C 서버 구현 시 반드시 챙길 항목

- `SO_REUSEADDR` 설정
- `SIGPIPE` 방지
- partial read 처리
- partial write 처리
- `Content-Length` 검증
- 잘못된 method/path/body 처리
- 요청 1건 처리 후 `close(fd)`
- request/response buffer의 메모리 소유권 정리
- `DbResult` 해제 함수 제공

## 권장 폴더 구조

최소 구현 기준으로 아래 정도만 잡는다.

```text
.
├── project/
│   ├── ai-context/
│   ├── personas/
│   └── plans/
├── src/
│   ├── main.c
│   ├── server/
│   ├── db/
│   └── concurrency/
├── third_party/
│   └── team7_engine/
├── tests/
├── Dockerfile
├── docker-compose.yml
├── Makefile
└── README.md
```

세부 파일 분리는 구현하면서 늘리되, 초기에 과하게 쪼개지 않는다.

## 역할별 작업 배치

### PM 단계

- 최소 기능 4개를 문서화한다.
  - `health-check`
  - `create-user`
  - `list-users`
  - `get-user-by-id`
- 각 기능의 acceptance criteria를 적는다.

### Architect 단계

- HTTP 범위를 최소 subset으로 확정한다.
- 메인 스레드와 worker 책임을 확정한다.
- DB 어댑터 경계를 확정한다.
- 락 전략을 문서화한다.
- Docker 실행 구조와 테스트 경로를 문서화한다.

### Developer 단계

- 구현 전 `project/ai-context`를 읽고 규칙을 적용한다.
- 서버보다 먼저 TEAM7 엔진 래핑 PoC를 만든다.
- 기능 단위로 커밋을 나눈다.
- 구현은 단순성과 데모 가능성을 우선한다.
- 동일 바이너리가 로컬과 Docker에서 모두 실행되도록 환경 파일을 추가한다.

### QA 단계

- 기능별 정상 케이스와 실패 케이스를 검증한다.
- malformed request와 동시 요청을 반드시 테스트한다.
- 실패 시 재현 절차를 남긴다.
- Docker 경로에서도 빌드와 smoke test가 가능한지 확인한다.

### Reviewer 단계

- `ai-context` 준수 여부를 본다.
- 과설계 여부를 본다.
- 커밋 단위가 적절한지 본다.
- 메모리, 동시성, 소켓 처리 위험을 우선 검토한다.

## 구현 단계

### 1단계. TEAM7 엔진 래핑 가능성 검증

- TEAM7 구조를 분석한다.
- 엔진 부분만 가져와 독립적으로 빌드해 본다.
- `INSERT`, `SELECT`, `SELECT WHERE id`가 동작하는지 확인한다.
- 서버에 맞게 수정이 필요한 지점을 목록화한다.

완료 기준:

- `db_execute_sql()` 형태의 래퍼 PoC가 동작한다.

### 2단계. DB 어댑터 최소 구현

- `db_init`, `db_execute_sql`, `db_shutdown` 구현
- `DbResult` 구조 정의
- 결과 해제 함수 정의

완료 기준:

- 소켓 없이도 DB 어댑터 단위 테스트가 가능하다.

### 3단계. `GET /health` 단독 서버 구현

- `socket`, `bind`, `listen`, `accept`
- `SO_REUSEADDR`
- 요청당 연결 1회 처리
- 고정 JSON 응답

완료 기준:

- `curl`로 `GET /health` 호출 시 정상 응답이 온다.

### 4단계. Thread Pool + Job Queue 연결

- worker 생성
- `client_fd` enqueue/dequeue
- worker가 read/parse/write/close 전체 처리

완료 기준:

- 여러 요청이 worker에서 처리되는 로그가 보인다.

### 5단계. 최소 HTTP 파싱 구현

- request line 파싱
- `GET`, `POST`만 지원
- `Content-Length` 기반 body 읽기
- malformed request에 400 응답

완료 기준:

- 잘못된 요청에도 서버가 죽지 않는다.

### 6단계. REST -> SQL -> DB 연동

- `POST /users`
- `GET /users`
- `GET /users/{id}`
- JSON 응답 생성

완료 기준:

- 세 개 endpoint가 실제 DB 엔진과 연결되어 동작한다.

### 7단계. 락 적용과 동시성 테스트

- `SELECT`에 read lock
- `INSERT`에 write lock
- 혼합 요청 테스트

완료 기준:

- 동시 요청에서 crash, race, corruption이 없다.

### 8단계. README와 데모 정리

- 실행 방법
- API 예제
- 동시 요청 테스트 명령
- Docker 실행 명령
- 구조 설명
- 한계와 제외 범위

완료 기준:

- 팀원이 문서만 보고 실행과 데모를 재현할 수 있다.

## Test Plan

### 우선 테스트

- `GET /health`
- `POST /users`
- `GET /users`
- `GET /users/{id}`
- Docker 환경에서의 빌드와 smoke test

### 실패 케이스

- 잘못된 method
- 잘못된 path
- malformed JSON
- 필드 누락
- 숫자가 아닌 `id`
- 존재하지 않는 `id`

### 동시성 케이스

- 동시 `GET /users`
- 동시 `GET /users/{id}`
- `POST /users`와 `GET /users` 혼합 요청

### 최소 합격 기준

- 서버가 죽지 않고 요청/응답이 왕복된다.
- thread pool이 실제로 요청을 처리한다.
- TEAM7 기반 엔진이 API 뒤에서 동작한다.
- README만으로 데모 가능하다.

## 발표 준비 포인트

- 왜 새 DB 엔진이 아니라 TEAM7 엔진 재사용을 택했는가
- 왜 TEAM7 코드를 필요 범위에서 수정했는가
- 왜 worker가 read/parse/execute/write/close를 모두 담당하는가
- 왜 coarse-grained lock으로 시작했는가
- 병렬 처리와 무결성을 어떻게 동시에 설명할 것인가
- 왜 Docker 경로를 추가했고, 팀 실행 환경 차이를 어떻게 줄였는가

## Assumptions

- TEAM7 엔진은 서버 연동에 필요한 범위에서 수정 가능하다.
- 저장소는 메모리 기반으로 유지한다.
- 외부 인터페이스는 REST API로 제한한다.
- 리소스는 `users` 단일 테이블만 지원한다.
- 성능 최적화보다 완성도와 데모 가능성을 우선한다.
