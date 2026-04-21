# Team7 엔진 기반 미니 DBMS REST API 서버 최소 구현 플랜

## 목적

이번 주 과제 요구사항에 맞춰, 이전 주차 TEAM7의 SQL 처리기와 B+Tree 엔진을 재사용한 뒤 그 위에 HTTP API 서버와 멀티스레드 처리 구조를 얹는다.

최소 목표는 새 DB 엔진을 만드는 것이 아니라, 기존 엔진을 안정적으로 서버화하고 외부 클라이언트가 API를 통해 DBMS 기능을 사용할 수 있게 만드는 것이다.

## 과제 요구사항 정리

- 구현 대상은 미니 DBMS API 서버다.
- 외부 클라이언트가 API를 통해 DB 기능을 사용할 수 있어야 한다.
- 스레드 풀을 두고 요청마다 작업을 분배해 병렬 처리해야 한다.
- 내부 DB 엔진은 이전 차수의 SQL 처리기와 B+Tree 구현을 재사용한다.
- 구현 언어는 C다.
- 결과물뿐 아니라 핵심 로직을 설명할 수 있어야 한다.
- 기본 구현 완료 후 차별화 요소를 하나 이상 검토한다.

## Summary

이 프로젝트는 이전 주차 TEAM7 레포 `week7-tem7-sql_processor_2`의 SQL 처리기와 B+Tree 엔진을 내부 DB 엔진으로 재사용하고, 그 위에 HTTP 기반 REST API 서버, Thread Pool, 작업 큐를 얹는 방식으로 최소 구현한다.

재사용 대상 레포:

- `https://github.com/Jungle-12-303/week7-tem7-sql_processor_2`

저장 엔진은 메모리 기반 재사용을 기본값으로 두고, 서버-엔진 연결, 동시성 제어, 테스트, README 데모 가능 상태까지를 이번 주 구현 범위로 잡는다.

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

## 범위 정의

### 이번 주 최소 구현 범위

- TEAM7 엔진을 라이브러리처럼 포함하거나 연결
- 단일 프로세스 HTTP 서버 구현
- 고정 크기 Thread Pool + 작업 큐 구현
- REST 요청을 내부 SQL로 변환하는 어댑터 계층 구현
- `users` 단일 테이블 기준 API 제공
- coarse-grained read-write lock 기반 동시성 제어
- 실행 방법, 구조도, 데모 흐름이 포함된 README 작성
- 역할별 산출물이 남는 문서 중심 개발 방식 적용

### 이번 주 제외 범위

- 새로운 저장 엔진 설계
- 복수 테이블 관리
- 트랜잭션
- 인증/인가
- HTTP keep-alive
- chunked encoding
- 정교한 락 분할
- 디스크 영속성 확장

## Key Changes

### 1. 재사용 대상 엔진 명시

- 기존 엔진은 TEAM7 레포 `week7-tem7-sql_processor_2`의 구현을 사용한다.
- 재사용 레포 URL은 아래로 고정한다.
  - `https://github.com/Jungle-12-303/week7-tem7-sql_processor_2`
- 재사용 대상 핵심 구성요소는 아래로 고정한다.
  - `Table`
  - `BPTree`
  - `sql_execute(Table *table, const char *input)`
  - `SQLResult`
- 서버는 TEAM7의 CLI 프로그램을 재사용하는 것이 아니라, TEAM7 엔진 코드를 라이브러리처럼 포함하거나 연결해 함수 호출 방식으로 사용한다.
- TEAM7의 단일 `users` 테이블 전제를 그대로 따른다.
- TEAM7 엔진이 지원하는 SQL 범위를 서버 내부 변환 기준으로 사용한다.
  - 최소 지원: `INSERT`, `SELECT`
  - 조건 조회: `id`, `name`, `age`

### 2. 서버 아키텍처

- 단일 프로세스 C 서버를 만든다.
- 네트워크 계층은 최소 구현 기준으로 `HTTP/1.1 + JSON` 요청/응답을 사용한다.
- 기본 실행 흐름은 아래로 고정한다.

```text
accept
-> request parse
-> job enqueue
-> worker thread execute
-> response write
```

- 메인 스레드는 소켓 accept와 연결 분배만 담당하고, SQL 실행은 worker thread만 수행한다.
- 고정 크기 Thread Pool과 작업 큐를 둔다.
- worker 수는 기본 `4` 또는 CPU 코어 수 기반 고정값으로 둔다.
- 구현 초기에는 단순성과 설명 가능성을 우선한다.

### 3. DB 엔진 연결 방식

- TEAM7 엔진을 감싸는 서버 전용 어댑터 계층을 둔다.
- 서버 계층에서 필요한 최소 엔진 인터페이스는 아래 3개로 통일한다.
  - `db_init(...)`
  - `db_execute_sql(const char *sql, DbResult *out)`
  - `db_shutdown(...)`
- `db_init(...)`는 내부적으로 TEAM7의 `table_create()`를 호출해 전역 `Table` 인스턴스를 준비한다.
- `db_execute_sql(...)`는 내부적으로 TEAM7의 `sql_execute(g_table, sql)`를 호출하고, 반환된 `SQLResult`를 서버용 `DbResult`로 변환한다.
- `db_shutdown(...)`는 내부적으로 `table_destroy()`와 결과 정리 로직을 호출한다.
- CLI 출력 중심 코드는 서버에서 직접 사용하지 않고, 결과를 구조체와 JSON 응답으로 변환하도록 어댑터 계층에서 흡수한다.
- B+Tree는 TEAM7 구현을 그대로 사용하되, `id` 기반 점조회와 조건 조회 시 실제 인덱스 경로가 사용되도록 유지한다.

### 4. 동시성 제어

- 전역 DB 인스턴스 1개를 두고 서버 전체가 공유한다.
- TEAM7 엔진은 기본적으로 thread-safe하지 않다는 전제로 서버 계층에서 보호한다.
- 최소 구현에서는 DB 전체 단위 read-write lock 1개로 시작한다.
- 기본 정책:
  - `SELECT`: read lock
  - `INSERT`: write lock
- finer-grained lock은 이번 최소 구현 범위에서 제외한다.
- 목표는 고성능 최적화보다 데이터 레이스 방지와 엔진 무결성 보장이다.
- 작업 큐는 producer-consumer 구조로 구현하고, `mutex + condition variable`을 사용한다.
- graceful shutdown 시에는 새 요청 수락 중단, 큐 drain, worker join 순서로 종료한다.

### 5. RESTful API 인터페이스

- API는 리소스 중심 REST 형식으로 설계한다.
- 최소 리소스는 `users` 단일 테이블로 고정한다.
- 외부에는 SQL 문자열이 아니라 REST endpoint를 노출하고, 서버 내부에서 요청을 TEAM7 엔진이 이해하는 SQL로 변환한다.
- 최소 엔드포인트는 아래로 고정한다.
  - `GET /health`
  - `POST /users`
  - `GET /users`
  - `GET /users/{id}`
- 엔드포인트별 동작:
  - `POST /users`
    - request body: `{ "name": "Alice", "age": 20 }`
    - 내부 변환: `INSERT INTO users VALUES ('Alice', 20);`
    - TEAM7 `sql_execute()` 호출
    - success: `201 Created`
    - response: `{ "ok": true, "id": 1, "message": "created" }`
  - `GET /users`
    - 내부 변환: `SELECT * FROM users;`
    - success: `200 OK`
    - response: `{ "ok": true, "rows": [...], "row_count": n }`
  - `GET /users/{id}`
    - 내부 변환: `SELECT * FROM users WHERE id = {id};`
    - 존재 시 `200 OK`
    - 없으면 `404 Not Found`
  - 선택 구현:
    - `GET /users?age=20`
    - `GET /users?name=Alice`
    - TEAM7 SQL 지원 범위 안에서만 매핑한다.
- 실패 응답은 JSON으로 통일한다.
  - validation 오류: `400 Bad Request`
  - 리소스 없음: `404 Not Found`
  - 내부 SQL 실행 실패: `500 Internal Server Error`

### 6. README와 데모 준비

- README는 발표 기준 문서로 작성한다.
- 반드시 포함할 항목:
  - TEAM7 엔진 재사용 사실과 이유
  - 재사용 레포 URL
  - TEAM7 엔진과 API 서버의 연결 구조
  - 전체 구조도
  - 요청 처리 흐름
  - Thread Pool 설명
  - 락 전략 설명
  - REST API 명세
  - REST -> SQL -> TEAM7 엔진 실행 흐름
  - 실행 방법
  - 테스트 방법
  - 동시 요청 데모 시나리오
- 발표 데모는 아래 흐름으로 고정한다.
  - 서버 실행
  - `POST /users`로 사용자 2~3명 생성
  - `GET /users`와 `GET /users/{id}` 확인
  - 동시 요청 테스트
  - TEAM7 엔진 재사용 방식과 테스트 결과 설명

### 7. 역할별 산출물 관리

- PM
  - 기능 정의 문서
  - acceptance criteria
  - 구현 항목 목록
- Architect
  - API 설계
  - 구조 정의
  - 데이터 흐름 및 락 전략
- Developer
  - 구현 코드
  - 변경 요약
  - PR 또는 커밋 단위 결과
- QA
  - 테스트 케이스
  - 검증 결과
  - 재현 절차
- Reviewer
  - 리뷰 코멘트
  - 구조/가독성/주석 개선 요청

## 권장 폴더 구조

최소 구현 기준으로 아래 구조를 목표로 둔다.

```text
.
├── project/
│   ├── ai-context/
│   ├── personas/
│   └── plans/
├── src/
│   ├── server/
│   │   ├── http_server.c
│   │   ├── http_server.h
│   │   ├── router.c
│   │   ├── router.h
│   │   ├── request_parser.c
│   │   ├── request_parser.h
│   │   ├── response_builder.c
│   │   └── response_builder.h
│   ├── concurrency/
│   │   ├── thread_pool.c
│   │   ├── thread_pool.h
│   │   ├── job_queue.c
│   │   ├── job_queue.h
│   │   ├── rwlock.c
│   │   └── rwlock.h
│   ├── db/
│   │   ├── db_adapter.c
│   │   ├── db_adapter.h
│   │   ├── sql_mapper.c
│   │   ├── sql_mapper.h
│   │   ├── result_mapper.c
│   │   └── result_mapper.h
│   ├── common/
│   │   ├── error.c
│   │   ├── error.h
│   │   ├── json.c
│   │   └── json.h
│   └── main.c
├── third_party/
│   └── team7_engine/
├── tests/
│   ├── unit/
│   ├── integration/
│   └── concurrency/
├── Makefile
└── README.md
```

구현 도중 실제 파일명은 바뀔 수 있지만, 책임 분리는 위 구조를 기준으로 맞춘다.

## 역할별 작업 배치

### PM 단계

- 기능을 작은 단위로 쪼개서 문서화한다.
- 예:
  - `health-check.md`
  - `create-user.md`
  - `list-users.md`
  - `get-user-by-id.md`
- 각 문서에 목표, 입력, 출력, acceptance criteria를 적는다.

### Architect 단계

- 기능 정의 문서를 읽고 API 명세와 내부 흐름을 설계한다.
- 서버, 라우터, DB 어댑터, 스레드 풀, 락 계층 경계를 고정한다.
- 요청이 어떤 자료구조를 거쳐 어떤 함수까지 흐르는지 정리한다.

### Developer 단계

- PM과 Architect 문서를 입력으로 삼아 구현한다.
- 구현 전 `project/ai-context`를 먼저 읽고 해당 규칙을 적용한다.
- 기능 단위로 브랜치/커밋/PR을 구성한다.
- 커밋은 리뷰 가능한 크기로 쪼개고, 서로 다른 성격의 변경은 분리한다.
- 핵심 로직은 설명 가능한 수준으로 주석과 구조를 유지한다.

### QA 단계

- 기능 정의 문서 기준으로 테스트 케이스를 만든다.
- 정상, 예외, 경계, 동시성 케이스를 검증한다.
- 실패하면 재현 절차와 기대 결과를 남겨 Developer에게 돌려준다.

### Reviewer 단계

- 코드와 테스트, 문서를 함께 본다.
- 버그 위험, 구조 문제, 주석 부족, 요구사항 누락을 우선 지적한다.
- `ai-context` 준수 여부와 커밋 단위 적절성도 함께 확인한다.
- 수정이 필요하면 다시 Developer 단계로 되돌린다.

## 구현 단계

### 1단계. PM 문서화

- 기능을 최소 단위로 나눠 문서화한다.
- 각 기능 문서에 완료 조건을 적는다.
- 이번 주 범위와 제외 범위를 확정한다.

### 2단계. Architect 설계

- API 명세와 요청/응답 형식을 확정한다.
- 모듈 경계와 처리 흐름을 정리한다.
- 동시성 전략과 락 정책을 문서화한다.

### 3단계. 재사용 엔진 확보

- TEAM7 레포 구조를 분석한다.
- 필요한 소스와 헤더를 현재 저장소에 포함하거나 서브디렉터리로 정리한다.
- CLI 의존 코드와 순수 엔진 코드를 분리한다.
- `table_create`, `sql_execute`, `table_destroy`, `SQLResult` 사용 경로를 확인한다.

### 4단계. Developer 구현 - DB 어댑터 계층

- `db_init`, `db_execute_sql`, `db_shutdown` 인터페이스를 만든다.
- TEAM7 `SQLResult`를 서버 전용 `DbResult`로 변환한다.
- 서버가 CLI 출력 포맷에 의존하지 않도록 한다.

### 5단계. Developer 구현 - HTTP 요청/응답 최소 골격

- 소켓 생성, bind, listen, accept를 구현한다.
- HTTP request line, header, body를 최소 수준으로 파싱한다.
- JSON body 파서와 응답 생성 유틸을 연결한다.

### 6단계. Developer 구현 - Thread Pool + 작업 큐

- 고정 worker 스레드 생성
- 작업 큐 enqueue/dequeue
- 대기와 종료를 위한 condition variable 연결
- 요청별 연결 정보를 job 단위로 넘기는 구조 설계

### 7단계. Developer 구현 - 라우팅과 REST -> SQL 변환

- `/health`, `/users`, `/users/{id}` 라우팅
- `POST /users` -> `INSERT`
- `GET /users` -> `SELECT *`
- `GET /users/{id}` -> `SELECT ... WHERE id = ...`
- 조건 조회가 가능하면 `age`, `name` 필터도 확장

### 8단계. Developer 구현 - 동시성 보호 적용

- 전역 DB에 read-write lock 적용
- 읽기/쓰기 요청별 lock 정책 분리
- 종료 시 worker와 큐 정리 로직 구현

### 9단계. QA 검증

- 테스트 케이스 문서를 기준으로 기능별 검증을 수행한다.
- 정상, 예외, 경계, 동시성 테스트를 실행한다.
- 실패 사례와 재현 절차를 기록한다.

### 10단계. Reviewer 검토와 수정 반복

- 코드, 테스트, 문서, 주석을 함께 리뷰한다.
- 수정 요청을 정리하고 필요한 경우 Developer 단계로 되돌린다.
- 리스크가 허용 범위 안으로 들어오면 완료로 본다.

### 11단계. 테스트와 데모 문서화

- 단위 테스트
- 통합 테스트
- 동시 요청 테스트
- README 실행 예제와 데모 시나리오 정리

## Test Plan

### 단위 테스트

- 작업 큐 enqueue/dequeue
- Thread Pool 초기화/종료
- REST 요청을 SQL 문자열로 변환하는 어댑터
- TEAM7 `SQLResult`를 서버용 `DbResult`와 JSON으로 변환하는 어댑터
- JSON 요청 파싱과 응답 생성

### 엔진 연동 테스트

- TEAM7 엔진 초기화 후 `INSERT INTO users VALUES ('Alice', 20);`
- `SELECT * FROM users;`
- `SELECT * FROM users WHERE id = 1;`
- 잘못된 SQL에 대한 TEAM7 에러 응답 변환 확인

### 통합 테스트

- `POST /users` 성공 확인
- `GET /users` 전체 조회 확인
- `GET /users/{id}` 단건 조회 확인
- 존재하지 않는 `id`에 대해 `404` 확인
- malformed JSON 또는 필드 누락에 대해 `400` 확인
- 동시 `GET /users/{id}` 요청 다건 처리 확인
- `POST /users`와 `GET /users` 동시 요청 시 무결성 확인

### 최소 합격 기준

- 서버가 죽지 않고 요청/응답이 정상 왕복된다.
- 멀티스레드 환경에서도 race condition 없이 동작한다.
- TEAM7 엔진이 API 서버 뒤에서 일관되게 동작한다.
- README만으로 실행과 데모가 가능하다.

## Assumptions

- 기존 엔진은 TEAM7 레포 `week7-tem7-sql_processor_2`를 사용한다.
- 재사용 레포 URL은 `https://github.com/Jungle-12-303/week7-tem7-sql_processor_2`다.
- 저장소는 TEAM7 구현 그대로 메모리 기반으로 유지한다.
- 외부 인터페이스는 REST API로 제한하고, SQL은 내부 변환용으로만 사용한다.
- 리소스는 `users` 단일 테이블만 지원한다.
- 동시성 전략은 coarse-grained read-write lock 1개로 시작한다.
- 성능 최적화보다 완성도, 안정성, 설명 가능성을 우선한다.
- 차별화 요소는 기본 구현 완료 후 아래 중 하나만 추가한다.
  - `GET /stats` 상태 API
  - 요청 처리 로그
  - 간단한 benchmark 결과 정리

## 리스크와 대응

- TEAM7 엔진이 CLI 출력 중심 구조일 수 있다.
  - 대응: 어댑터 계층에서 결과 구조체 변환을 우선 구현한다.
- TEAM7 엔진 내부가 thread-safe하지 않을 수 있다.
  - 대응: 서버 계층에서 read-write lock으로 보호한다.
- HTTP 파서 구현이 길어질 수 있다.
  - 대응: 최소 스펙만 지원하고 복잡한 기능은 제외한다.
- JSON 파싱과 문자열 조합에서 버그가 나기 쉽다.
  - 대응: 입력 검증과 버퍼 길이 체크를 기본 규칙으로 둔다.

## 발표용 핵심 설명 포인트

- 왜 새 DB 엔진이 아니라 기존 엔진 재사용을 선택했는가
- 서버와 엔진 사이를 왜 어댑터 계층으로 분리했는가
- 왜 coarse-grained lock으로 시작했는가
- 요청이 메인 스레드에서 바로 실행되지 않고 큐를 거치는 이유는 무엇인가
- REST 요청이 내부 SQL로 어떻게 변환되는가
- 병렬 처리 상황에서 어떤 무결성을 보장하는가
