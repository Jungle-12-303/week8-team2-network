# 7단계. 4분 발표 스크립트 암기하기

## 목표

4분 안에 과제 목표, 아키텍처, 요청 흐름, 동시성 설계, 데모, 테스트와 개선 방향을 말할 수 있게 준비합니다.

## 예상 소요 시간

- 40분

## 발표 전체 구조

- [ ] 0:00~0:30 과제 목표와 한 문장 소개
- [ ] 0:30~1:10 전체 아키텍처
- [ ] 1:10~2:00 요청 처리 흐름
- [ ] 2:00~2:45 동시성 설계
- [ ] 2:45~3:30 터미널 데모
- [ ] 3:30~4:00 테스트 결과와 개선 방향

## 7-1. 0:00~0:30 과제 목표와 한 문장 소개

외울 문장:

```text
저희 프로젝트는 기존 REPL 기반 SQL 엔진을 HTTP API 서버로 감싼 미니 DBMS입니다. 클라이언트는 POST /query로 SQL 문자열을 보내고, 서버는 기존 SQL 처리기와 B+Tree 기반 Table에 실행한 뒤 JSON으로 응답합니다.
```

조금 더 짧은 버전:

```text
한 문장으로 말하면, REPL 기반 SQL 엔진을 C HTTP API 서버로 감싸 외부 클라이언트가 SQL을 실행할 수 있게 만든 프로젝트입니다.
```

반드시 포함할 키워드:

- [ ] 기존 SQL 엔진
- [ ] B+Tree
- [ ] HTTP API 서버
- [ ] `POST /query`
- [ ] JSON 응답

체크 질문:

- [ ] 이 프로젝트는 SQL 엔진을 새로 만든 것인가, 기존 엔진을 API로 감싼 것인가?
- [ ] 외부 클라이언트는 어떤 endpoint를 호출하는가?

## 7-2. 0:30~1:10 전체 아키텍처 설명

외울 문장:

```text
구조는 세 계층입니다. 첫 번째는 TCP/HTTP 요청을 받는 API 서버, 두 번째는 connection을 worker에게 나눠주는 thread pool, 세 번째는 기존 SQL 처리기와 B+Tree를 포함한 DB 엔진입니다.
```

추가 설명:

```text
API 서버는 네트워크 요청을 받고, thread pool은 여러 요청을 worker에게 분배하고, DB 엔진은 실제 SQL 파싱과 Table/B+Tree 조회 및 삽입을 담당합니다.
```

반드시 포함할 키워드:

- [ ] API 서버
- [ ] thread pool
- [ ] SQL 처리기
- [ ] Table
- [ ] B+Tree
- [ ] JSON 직렬화

화이트보드 없이 말할 구조:

```text
client -> API server -> thread pool worker -> sql_execute -> Table/B+Tree -> JSON response
```

체크 질문:

- [ ] API 서버와 SQL 엔진의 책임은 어떻게 다른가?
- [ ] thread pool은 어느 계층 사이에 있는가?

## 7-3. 1:10~2:00 요청 처리 흐름 설명

외울 문장:

```text
요청 흐름은 accept -> queue -> worker -> HTTP parser -> api_handle_query -> sql_execute -> JSON response입니다. 메인 스레드는 connection을 accept하고 queue에 넣고, worker thread가 실제 요청을 읽고 SQL을 실행합니다.
```

조금 더 자세한 버전:

```text
server_run의 accept loop가 client fd를 받고 thread_pool_submit으로 bounded queue에 넣습니다. worker는 queue에서 fd를 꺼내 http_read_request로 POST /query와 Content-Length를 확인하고, body의 SQL 문자열을 api_handle_query로 넘깁니다. api_handle_query는 DB mutex를 잡고 sql_execute를 호출한 뒤 SQLResult를 JSON으로 바꿔 응답합니다.
```

반드시 포함할 함수 이름:

- [ ] `server_run`
- [ ] `accept`
- [ ] `thread_pool_submit`
- [ ] `http_read_request`
- [ ] `api_handle_query`
- [ ] `sql_execute`
- [ ] `http_send_response`

체크 질문:

- [ ] 메인 스레드가 SQL을 직접 실행하는가?
- [ ] worker는 client fd를 어디에서 받는가?
- [ ] SQL 문자열은 HTTP 요청의 어디에 들어 있는가?

## 7-4. 2:00~2:45 동시성 설계 설명

외울 문장:

```text
동시성은 thread pool과 DB mutex로 처리했습니다. worker는 여러 개라 요청 처리는 병렬로 받을 수 있지만, 공유 Table은 thread-safe하지 않기 때문에 SQL 실행 구간은 mutex로 보호했습니다. queue는 bounded queue라 과부하 시 503을 반환할 수 있습니다.
```

추가 설명:

```text
thread pool queue는 head, tail, size를 가진 circular queue이고, worker는 queue가 비어 있으면 condition variable로 대기합니다. 새 connection이 들어오면 submit이 condition signal로 worker를 깨웁니다.
```

반드시 포함할 키워드:

- [ ] worker 4개
- [ ] queue capacity 16
- [ ] bounded queue
- [ ] circular queue
- [ ] mutex
- [ ] condition variable
- [ ] DB mutex
- [ ] 503 queue full

체크 질문:

- [ ] 왜 thread-per-request 구조를 쓰지 않았는가?
- [ ] 왜 DB mutex를 썼는가?
- [ ] 이 서버는 어떤 부분이 병렬이고 어떤 부분이 직렬인가?

## 7-5. 2:45~3:30 터미널 데모 스크립트

데모 전 준비:

- [ ] 서버가 실행 중인지 확인한다.
- [ ] curl 명령을 복사할 수 있게 준비한다.
- [ ] 너무 긴 출력은 필요한 부분만 설명한다.

데모 순서:

- [ ] smoke test
- [ ] INSERT
- [ ] SELECT
- [ ] `WHERE id >= 1`
- [ ] 동시 INSERT
- [ ] 최종 SELECT

말하면서 실행할 문장:

```text
먼저 smoke test로 INSERT와 SELECT가 연결되는지 확인합니다.
```

```text
이제 직접 INSERT를 보내면 JSON으로 inserted_id와 row_count가 반환됩니다.
```

```text
SELECT 결과는 row_count와 rows 배열로 내려옵니다.
```

```text
id 조건 조회는 B+Tree index를 사용하는 경로입니다.
```

```text
동시 INSERT는 xargs -P8로 여러 요청을 동시에 보내고, DB mutex 덕분에 inserted_id가 중복되지 않는지 확인합니다.
```

주의:

- [ ] 동시 INSERT 출력 순서가 섞일 수 있다.
- [ ] 출력이 길면 `row_count`와 `inserted_id` 중심으로 설명한다.
- [ ] 시간이 부족하면 동시 INSERT는 명령과 검증 결과만 말한다.

## 7-6. 3:30~4:00 테스트 결과와 개선 방향

외울 문장:

```text
SQL 엔진은 unit test로 검증했고, API 서버는 smoke test와 curl 요청으로 검증했습니다. 동시 INSERT 요청에서도 ID가 중복되지 않고 서버가 유지되는 것을 확인했습니다.
```

개선 방향 문장:

```text
현재 한계는 in-memory only, 단일 users table, 제한된 SQL 문법, 전역 DB mutex입니다. 개선한다면 rwlock으로 SELECT 병렬성을 높이고, table을 여러 개 지원하며, persistence를 추가할 수 있습니다.
```

반드시 포함할 테스트:

- [ ] `make clean && make db_server`
- [ ] `make unit_test && ./unit_test`
- [ ] `scripts/smoke_test.sh`
- [ ] 동시 INSERT
- [ ] 오류 SQL JSON 응답

체크 질문:

- [ ] unit test는 무엇을 검증하는가?
- [ ] smoke test는 무엇을 검증하는가?
- [ ] concurrency test는 무엇을 검증하는가?

## 7-7. 4분 발표 전체 원고

아래 원고를 먼저 그대로 읽고, 이후 자신의 말투로 바꿉니다.

```text
저희 프로젝트는 기존 REPL 기반 SQL 엔진을 HTTP API 서버로 감싼 미니 DBMS입니다. 클라이언트는 POST /query로 SQL 문자열을 보내고, 서버는 기존 SQL 처리기와 B+Tree 기반 Table에 실행한 뒤 JSON으로 응답합니다.

구조는 세 계층입니다. 첫 번째는 TCP/HTTP 요청을 받는 API 서버, 두 번째는 connection을 worker에게 나눠주는 thread pool, 세 번째는 기존 SQL 처리기와 B+Tree를 포함한 DB 엔진입니다.

요청 흐름은 accept -> queue -> worker -> HTTP parser -> api_handle_query -> sql_execute -> JSON response입니다. server_run의 accept loop가 client fd를 받고 thread_pool_submit으로 queue에 넣습니다. worker는 queue에서 fd를 꺼내 http_read_request로 POST /query와 Content-Length를 확인하고, body의 SQL 문자열을 api_handle_query로 넘깁니다. api_handle_query는 DB mutex를 잡고 sql_execute를 호출한 뒤 SQLResult를 JSON으로 바꿔 응답합니다.

동시성은 thread pool과 DB mutex로 처리했습니다. worker는 여러 개라 HTTP 요청 파싱과 응답 처리는 병렬로 수행될 수 있습니다. 다만 공유 Table과 B+Tree는 thread-safe하지 않기 때문에 SQL 실행 구간은 mutex로 보호했습니다. queue는 bounded queue라 과부하 시 503을 반환할 수 있습니다.

데모에서는 먼저 smoke test로 INSERT와 SELECT가 연결되는지 확인하고, 직접 SELECT와 WHERE id 조건 조회를 보여드리겠습니다. id 조건 조회는 B+Tree index를 사용하는 경로이고, name과 age 조건은 별도 index가 없어 선형 탐색입니다. 동시 INSERT는 여러 요청을 동시에 보내도 inserted_id가 중복되지 않는지 확인하는 방식으로 검증했습니다.

테스트는 SQL 엔진 unit test, API smoke test, curl 기반 기능 테스트로 나눠 진행했습니다. 현재 한계는 in-memory only, 단일 users table, 제한된 SQL 문법, 전역 DB mutex입니다. 개선한다면 rwlock으로 SELECT 병렬성을 높이고, multi-table과 persistence를 추가할 수 있습니다.
```

## 7-8. 시간 줄이기 버전

3분 안에 줄여야 할 때:

- [ ] B+Tree 내부 설명을 줄인다.
- [ ] HTTP 오류 설명을 생략한다.
- [ ] 데모는 smoke test와 `WHERE id >= 1`만 보여준다.
- [ ] 동시 INSERT는 명령과 검증 결과만 말한다.

짧은 원고:

```text
이 프로젝트는 기존 REPL 기반 SQL 엔진을 HTTP API 서버로 감싼 미니 DBMS입니다. 클라이언트는 POST /query로 SQL 문자열을 보내고, 서버는 sql_execute로 실행한 뒤 JSON 응답을 반환합니다.

구조는 API 서버, thread pool, DB 엔진 세 계층입니다. 메인 스레드는 accept한 client fd를 queue에 넣고, worker thread가 HTTP 요청을 읽고 api_handle_query를 호출합니다. api_handle_query는 DB mutex를 잡고 sql_execute를 실행한 뒤 SQLResult를 JSON으로 바꿉니다.

동시성은 worker thread 여러 개와 bounded queue로 처리하고, shared Table과 B+Tree는 DB mutex로 보호합니다. id 조회는 B+Tree index를 사용하고, name과 age는 선형 탐색입니다.

테스트는 SQL unit test, smoke test, 동시 INSERT로 검증했습니다. 한계는 in-memory only, 단일 users table, 제한된 SQL 문법, 전역 DB mutex이며, 개선 방향은 rwlock, multi-table, persistence입니다.
```

## 7-9. 발표 연습 체크

- [ ] 원고를 보면서 1회 읽었다.
- [ ] 원고 없이 1회 말해봤다.
- [ ] 4분 타이머를 켜고 1회 완주했다.
- [ ] 데모 명령을 실제로 실행하며 1회 연습했다.
- [ ] 동시 INSERT 출력이 섞여도 설명할 수 있다.
- [ ] Q&A로 넘어가기 전 한계를 먼저 말했다.

## 완료 기준

- [ ] 4분 발표를 시간 안에 끝낼 수 있다.
- [ ] `accept -> queue -> worker -> sql_execute -> JSON` 흐름을 자연스럽게 말할 수 있다.
- [ ] thread pool과 DB mutex를 30초 안에 설명할 수 있다.
- [ ] 테스트 결과와 개선 방향을 말할 수 있다.
- [ ] 데모 중 출력이 예상과 조금 달라도 당황하지 않고 설명할 수 있다.
