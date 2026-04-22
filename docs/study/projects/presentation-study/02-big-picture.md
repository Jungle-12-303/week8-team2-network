# 2단계. 큰그림 문서 읽기

## 목표

코드 세부 구현에 들어가기 전에 프로젝트의 목적, 범위, 계층 구조, 발표 메시지를 정리합니다. 이 단계에서는 “왜 이렇게 만들었는가”에 답할 수 있어야 합니다.

## 예상 소요 시간

- 30~40분

## 읽을 파일

- [ ] `README.md`
- [ ] `AGENTS.md`
- [ ] `docs/study/projects/project-overview.md`
- [ ] 시간이 남으면 `docs/plan/02-server-architecture.md`
- [ ] 시간이 남으면 `docs/plan/03-api-contract.md`
- [ ] 시간이 남으면 `docs/plan/04-thread-pool-and-concurrency.md`
- [ ] 시간이 남으면 `docs/plan/05-db-engine-integration.md`

## 2-1. README에서 사용자 관점 파악

- [ ] `README.md`의 제목과 첫 설명을 읽는다.
- [ ] “What is implemented” 목록을 읽는다.
- [ ] Build, Run, API, Smoke test 섹션을 읽는다.
- [ ] Supported SQL 목록을 확인한다.

정리할 내용:

- [ ] 이 프로젝트는 어떤 프로그램인가?
- [ ] 외부 클라이언트는 어떤 endpoint를 호출하는가?
- [ ] 요청 body는 어떤 형식인가?
- [ ] 응답은 어떤 형식인가?
- [ ] 어떤 SQL 문법을 지원하는가?

메모:

```text
프로젝트 이름:
주요 endpoint:
요청 body 형식:
응답 형식:
지원 SQL:
```

체크 질문:

- [ ] README만 보고도 서버를 실행할 수 있는가?
- [ ] README만 보고도 API를 호출할 수 있는가?

## 2-2. AGENTS에서 과제 요구사항 파악

- [ ] `AGENTS.md`를 읽는다.
- [ ] 과제 목표를 표시한다.
- [ ] 구현 언어를 표시한다.
- [ ] thread pool 요구사항을 표시한다.
- [ ] 기존 SQL 처리기와 B+Tree 재사용 요구사항을 표시한다.
- [ ] 품질 요구사항을 표시한다.

요구사항 체크:

- [ ] API 서버를 구현해야 한다.
- [ ] 외부 클라이언트가 DBMS 기능을 사용할 수 있어야 한다.
- [ ] thread pool로 SQL 요청을 병렬 처리해야 한다.
- [ ] 이전 SQL 처리기와 B+Tree를 활용해야 한다.
- [ ] 구현 언어는 C다.
- [ ] 단위 테스트와 기능 테스트를 준비해야 한다.

체크 질문:

- [ ] REPL 프로그램만 있으면 과제 요구사항을 만족하는가?
- [ ] 왜 API 서버가 필요한가?
- [ ] 왜 기존 SQL 처리기와 B+Tree를 버리지 않고 재사용하는가?

답변 힌트:

```text
과제 요구사항은 외부 클라이언트가 DBMS 기능을 사용할 수 있는 API 서버입니다. 기존 REPL은 사람이 터미널에서 직접 입력하는 구조이므로 외부 API 요구사항을 만족하지 못합니다.
```

## 2-3. project-overview에서 발표 메시지 잡기

- [ ] `docs/study/projects/project-overview.md`의 한 문장 요약을 읽는다.
- [ ] “프로젝트가 해결하는 문제”를 읽는다.
- [ ] “큰 흐름” 코드를 읽는다.
- [ ] “핵심 구성요소” 4개를 확인한다.
- [ ] “발표자가 기억할 핵심 문장”을 읽는다.

핵심 구성요소:

- [ ] API 서버
- [ ] 스레드 풀
- [ ] 공유 DB 엔진
- [ ] 응답 변환

체크 질문:

- [ ] API 서버의 책임은 무엇인가?
- [ ] 스레드 풀의 책임은 무엇인가?
- [ ] 공유 DB 엔진의 책임은 무엇인가?
- [ ] 응답 변환 계층은 왜 필요한가?

답변 힌트:

```text
API 서버는 네트워크 요청을 받고, thread pool은 요청 처리 worker를 관리하고, DB 엔진은 SQL을 실행하고, 응답 변환 계층은 SQLResult를 JSON으로 바꿉니다.
```

## 2-4. 전체 구조를 3계층으로 나누기

아래 구조를 직접 손으로 적거나 팀원에게 말해봅니다.

```text
외부 클라이언트
  -> HTTP API 서버
  -> thread pool worker
  -> SQL 처리기
  -> Table / B+Tree
  -> SQLResult
  -> JSON HTTP response
```

3계층 설명:

- [ ] 네트워크 계층: socket, HTTP parser, response
- [ ] 동시성 계층: thread pool, bounded queue, worker
- [ ] DB 계층: SQL parser, Table, B+Tree

체크 질문:

- [ ] `server` 디렉터리는 주로 어떤 계층인가?
- [ ] `sql_processor` 디렉터리는 주로 어떤 계층인가?
- [ ] `scripts` 디렉터리는 어떤 용도인가?

## 2-5. 발표용 핵심 메시지 만들기

아래 문장을 3번 읽고, 보지 않고 말해봅니다.

```text
기존 메모리 기반 SQL 처리기와 B+Tree 인덱스를 내부 DB 엔진으로 두고, 그 앞에 C 기반 HTTP API 서버와 스레드 풀을 붙여 외부 클라이언트가 SQL 요청을 병렬로 실행할 수 있게 만든 프로젝트입니다.
```

더 짧은 버전:

```text
REPL 기반 SQL 엔진을 HTTP API 서버로 감싼 미니 DBMS입니다.
```

더 기술적인 버전:

```text
TCP socket 기반 HTTP 서버가 POST /query 요청을 받아 thread pool worker에 넘기고, worker가 기존 sql_execute 함수를 호출한 뒤 SQLResult를 JSON으로 직렬화해 반환합니다.
```

체크 질문:

- [ ] 비전공자에게 설명할 짧은 버전은 무엇인가?
- [ ] 평가자에게 설명할 기술 버전은 무엇인가?
- [ ] README 첫 문장으로 적합한 버전은 무엇인가?

## 2-6. 구현 범위와 한계 구분

구현된 것:

- [ ] C 기반 HTTP 서버 entrypoint
- [ ] `POST /query`
- [ ] raw socket HTTP request parsing
- [ ] `Content-Length` 기반 body 읽기
- [ ] bounded worker thread pool
- [ ] 기존 SQL 처리기 호출
- [ ] B+Tree 기반 id 조회
- [ ] JSON 응답 직렬화
- [ ] graceful shutdown flag
- [ ] Docker build and run support

현재 한계:

- [ ] in-memory only
- [ ] 단일 `users` table
- [ ] 제한된 SQL 문법
- [ ] 전역 DB mutex로 DB 실행 구간 직렬화
- [ ] persistence 없음
- [ ] transaction 없음
- [ ] authentication 없음
- [ ] production-grade HTTP parser는 아님

체크 질문:

- [ ] 한계를 말할 때 방어적으로 들리지 않게 설명하려면 어떻게 말해야 하는가?

답변 힌트:

```text
과제의 핵심은 기존 DB 엔진을 API 서버로 노출하고 동시 요청을 처리하는 것이어서, 안정성을 위해 단순한 in-memory 구조와 전역 mutex를 선택했습니다. 개선한다면 rwlock, persistence, multi-table을 추가할 수 있습니다.
```

## 2-7. 팀 공통 용어 통일

발표에서 용어가 섞이지 않도록 아래 표현으로 통일합니다.

- [ ] “API 서버”: HTTP 요청을 받는 서버 계층
- [ ] “thread pool”: worker thread와 job queue를 관리하는 동시성 계층
- [ ] “worker”: queue에서 client fd를 꺼내 요청을 처리하는 thread
- [ ] “SQL 엔진”: `sql_processor`의 `sql_execute`
- [ ] “Table”: in-memory `users` table
- [ ] “B+Tree”: `id` 기반 primary key index
- [ ] “DB mutex”: 공유 Table과 B+Tree를 보호하는 mutex
- [ ] “JSON 직렬화”: `SQLResult`를 HTTP response body로 바꾸는 작업

체크 질문:

- [ ] thread pool과 DB mutex는 같은 mutex인가?
- [ ] HTTP parser와 SQL parser는 같은 parser인가?
- [ ] API 서버가 SQL을 직접 실행하는가, 아니면 SQL 엔진에 위임하는가?

## 발표용 핵심 문장

```text
이번 프로젝트는 SQL 처리기를 새로 만드는 것이 아니라, 기존 DB 엔진을 HTTP API 서버로 외부에 열어주는 작업입니다.
```

```text
API 서버, thread pool, DB 엔진을 분리했기 때문에 각 계층의 책임을 설명하기 쉽습니다.
```

```text
테스트는 기존 SQL 단위 테스트, API 기능 테스트, 동시 요청 테스트로 나눠 검증합니다.
```

## 완료 기준

- [ ] README에서 실행 방법과 API 계약을 설명할 수 있다.
- [ ] AGENTS의 과제 요구사항을 설명할 수 있다.
- [ ] project overview의 큰 흐름을 말할 수 있다.
- [ ] 프로젝트를 10초, 30초, 1분 버전으로 설명할 수 있다.
- [ ] 구현된 것과 현재 한계를 구분해서 말할 수 있다.
- [ ] API 서버, thread pool, DB 엔진의 역할을 구분할 수 있다.
