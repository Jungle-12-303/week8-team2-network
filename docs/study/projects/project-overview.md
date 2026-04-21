# 미니 DBMS API 서버 프로젝트 큰그림

이 문서는 발표자가 프로젝트의 전체 그림을 빠르게 잡기 위한 개요 문서입니다. 세부 구현 순서는 [../../plan/00-plan-manager.md](../../plan/00-plan-manager.md)를 기준으로 보고, 이 문서는 “왜 이 프로젝트를 이렇게 만들었는가”를 설명하는 지도처럼 사용합니다.

## 한 문장 요약

기존 메모리 기반 SQL 처리기와 B+Tree 인덱스를 내부 DB 엔진으로 두고, 그 앞에 C 기반 HTTP API 서버와 스레드 풀을 붙여 외부 클라이언트가 SQL 요청을 병렬로 실행할 수 있게 만드는 프로젝트입니다.

## 프로젝트가 해결하는 문제

기존 `sql_processor`는 REPL에서 직접 SQL을 입력해 실행하는 구조입니다. 이번 과제는 이 엔진을 외부 클라이언트가 사용할 수 있도록 서버로 감싸는 것이 핵심입니다.

- 사용자는 터미널 REPL이 아니라 HTTP API로 SQL을 보냅니다.
- 서버는 요청을 받아 SQL 문자열을 꺼냅니다.
- worker thread가 기존 DB 엔진에 SQL을 실행합니다.
- 결과는 HTTP 응답으로 클라이언트에게 돌아갑니다.

## 큰 흐름

```text
외부 클라이언트
  -> HTTP POST /query
  -> API 서버 accept loop
  -> job queue
  -> worker thread
  -> HTTP body에서 SQL 추출
  -> sql_execute(shared_table, sql)
  -> Table / B+Tree 조회 또는 삽입
  -> SQLResult
  -> JSON HTTP response
```

## 핵심 구성요소

### 1. API 서버

외부 클라이언트와 만나는 입구입니다. TCP 연결을 받고 HTTP 요청을 읽은 뒤, `POST /query` 요청인지 확인합니다.

발표 포인트:

- API 서버는 DB 로직을 직접 구현하지 않습니다.
- 서버의 책임은 네트워크 요청을 받아 SQL 엔진에 전달하고 응답으로 바꾸는 것입니다.

### 2. 스레드 풀

여러 클라이언트 요청을 동시에 처리하기 위한 구조입니다. 요청마다 새 스레드를 만들지 않고, 미리 만들어둔 worker thread가 job queue에서 연결을 가져가 처리합니다.

발표 포인트:

- thread-per-request보다 생성 비용을 줄이고 구조가 안정적입니다.
- bounded queue로 서버가 감당할 수 있는 요청 수를 제한합니다.
- worker는 독립적으로 HTTP 요청을 읽고 응답을 만듭니다.

### 3. 공유 DB 엔진

서버 안에는 하나의 `Table` 인스턴스가 있고, 모든 요청이 이 테이블을 공유합니다. 기존 SQL 처리기의 `sql_execute(Table *table, const char *input)`를 그대로 사용합니다.

발표 포인트:

- 기존 SQL 파서와 B+Tree 인덱스를 재사용했습니다.
- `id` 기반 조회는 B+Tree를 활용하고, `name`, `age` 조건은 기존 구현처럼 선형 탐색을 사용합니다.
- DB 엔진은 thread-safe하지 않으므로 DB mutex로 보호합니다.

### 4. 응답 변환

`sql_execute()`의 결과인 `SQLResult`를 HTTP JSON 응답으로 변환합니다.

발표 포인트:

- INSERT 결과는 inserted id와 row count로 보여줍니다.
- SELECT 결과는 rows 배열로 보여줍니다.
- SQL 문법 오류는 서버 crash가 아니라 오류 JSON으로 반환합니다.

## 발표자가 기억할 핵심 문장

- “이번 프로젝트는 SQL 처리기를 새로 만드는 것이 아니라, 기존 DB 엔진을 API 서버로 외부에 열어주는 작업입니다.”
- “요청 처리 병렬성은 스레드 풀과 job queue로 만들고, 데이터 일관성은 DB mutex로 지킵니다.”
- “API 서버, thread pool, DB 엔진을 분리했기 때문에 각 계층의 책임을 설명하기 쉽습니다.”
- “테스트는 기존 SQL 단위 테스트, API 기능 테스트, 동시 요청 테스트로 나눠 검증합니다.”

## 플랜 문서와 연결

- [../../plan/00-plan-manager.md](../../plan/00-plan-manager.md): 전체 구현 순서와 문서 참조 순서.
- [../../plan/02-server-architecture.md](../../plan/02-server-architecture.md): 서버와 계층 구조.
- [../../plan/03-api-contract.md](../../plan/03-api-contract.md): 클라이언트가 호출할 API.
- [../../plan/04-thread-pool-and-concurrency.md](../../plan/04-thread-pool-and-concurrency.md): 병렬 처리와 DB lock.
- [../../plan/05-db-engine-integration.md](../../plan/05-db-engine-integration.md): 기존 SQL 처리기 연결 방식.
- [../../plan/08-demo-and-readme-plan.md](../../plan/08-demo-and-readme-plan.md): 발표 흐름과 데모 시나리오.

## 코드 작성 주석 원칙

이 프로젝트의 코드는 발표자가 읽고 설명할 수 있어야 합니다. 따라서 구현할 때는 핵심 흐름과 의도를 한국어 주석으로 친절하게 남깁니다.

- accept loop가 connection fd를 queue에 넘기는 이유.
- worker thread가 어떤 순서로 요청을 처리하는지.
- DB mutex를 잡는 이유와 lock 범위.
- `SQLResult`를 JSON으로 변환할 때 메모리 소유권이 어디에 있는지.
- 오류 응답을 HTTP 오류와 SQL 오류로 나누는 이유.

## 추천 발표 순서

1. 기존 SQL 처리기와 B+Tree가 이미 있는 상태에서 출발했다고 설명합니다.
2. 이번 과제의 목표가 “외부 클라이언트가 API로 DBMS 기능을 쓰게 만드는 것”이라고 말합니다.
3. HTTP 요청이 thread pool worker를 거쳐 `sql_execute()`까지 가는 흐름을 보여줍니다.
4. 공유 DB 엔진을 DB mutex로 보호한 이유를 설명합니다.
5. curl 데모와 테스트 결과로 구현이 실제로 동작함을 보여줍니다.
