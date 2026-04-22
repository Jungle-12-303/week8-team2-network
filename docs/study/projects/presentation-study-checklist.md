# 미니 DBMS API 서버 학습 및 발표 준비 체크리스트

이 체크리스트는 3~5시간 안에 프로젝트 구조를 이해하고, README와 터미널 데모 중심의 4분 발표 및 1분 Q&A를 준비하기 위한 순서입니다.

## 최종 목표

- [ ] 프로젝트를 한 문장으로 설명할 수 있다.
- [ ] HTTP 요청 하나가 서버 안에서 어떤 순서로 처리되는지 설명할 수 있다.
- [ ] thread pool, bounded queue, DB mutex를 왜 사용했는지 설명할 수 있다.
- [ ] 기존 SQL 처리기와 B+Tree가 API 서버에 어떻게 연결되는지 설명할 수 있다.
- [ ] 터미널에서 빌드, 실행, smoke test, 동시 요청 데모를 직접 할 수 있다.

## 상세 단계 문서

- [ ] [1단계. 실행 결과 먼저 확인하기](presentation-study/01-run-first.md)
- [ ] [2단계. 큰그림 문서 읽기](presentation-study/02-big-picture.md)
- [ ] [3단계. 요청 흐름 코드 추적하기](presentation-study/03-request-flow.md)
- [ ] [4단계. 동시성 구조 이해하기](presentation-study/04-concurrency.md)
- [ ] [5단계. DB 엔진 연결 이해하기](presentation-study/05-db-engine.md)
- [ ] [6단계. 테스트와 데모 준비하기](presentation-study/06-test-demo.md)
- [ ] [7단계. 4분 발표 스크립트 암기하기](presentation-study/07-presentation-script.md)
- [ ] [8단계. 팀원별 Q&A 백업 나누기](presentation-study/08-team-qa.md)

## 0단계. 시작 전 준비

- [ ] 프로젝트 루트로 이동한다.

```bash
cd /Users/juhoseok/Desktop/week8-team2-network
```

- [ ] 현재 프로젝트 파일 구조를 확인한다.

```bash
find . -maxdepth 2 -type f | sort
```

- [ ] 발표 핵심 문장을 먼저 외운다.

```text
REPL 기반 SQL 엔진을 C HTTP API 서버로 감싸서 외부 클라이언트가 SQL 요청을 병렬로 실행할 수 있게 만든 미니 DBMS입니다.
```

완료 기준:

- [ ] `server`, `sql_processor`, `scripts`, `docs` 디렉터리의 역할을 대략 말할 수 있다.

## 1단계. 실행 결과 먼저 확인하기

목표: 코드 읽기 전에 완성된 동작을 눈으로 확인한다.

- [ ] 서버를 빌드한다.

```bash
make clean && make db_server
```

- [ ] 서버를 실행한다.

```bash
./db_server 8080
```

- [ ] 다른 터미널에서 smoke test를 실행한다.

```bash
sh scripts/smoke_test.sh 8080
```

- [ ] 직접 INSERT 요청을 보낸다.

```bash
curl -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "INSERT INTO users VALUES ('Alice', 20);"
```

- [ ] 직접 SELECT 요청을 보낸다.

```bash
curl -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "SELECT * FROM users;"
```

완료 기준:

- [ ] `POST /query`가 SQL 문자열을 받는다는 것을 설명할 수 있다.
- [ ] 응답이 JSON으로 내려온다는 것을 확인했다.
- [ ] INSERT 응답의 `inserted_id`, `row_count` 의미를 설명할 수 있다.
- [ ] SELECT 응답의 `rows`, `row_count` 의미를 설명할 수 있다.

발표용 문장:

```text
클라이언트는 HTTP POST /query로 SQL 문자열을 보내고, 서버는 기존 SQL 엔진에 실행시킨 뒤 JSON으로 응답합니다.
```

## 2단계. 큰그림 문서 읽기

목표: 프로젝트가 왜 이런 구조인지 먼저 잡는다.

- [ ] `README.md`를 읽는다.
- [ ] `AGENTS.md`를 읽는다.
- [ ] `docs/study/projects/project-overview.md`를 읽는다.

확인 질문:

- [ ] 이 프로젝트의 과제 목표는 무엇인가?
- [ ] 기존 `sql_processor`는 어떤 역할인가?
- [ ] API 서버는 어떤 역할인가?
- [ ] 왜 REPL만으로는 과제 요구사항을 만족할 수 없는가?
- [ ] 왜 thread pool이 필요한가?

완료 기준:

- [ ] 프로젝트를 30초 안에 설명할 수 있다.
- [ ] API 서버, thread pool, SQL 엔진의 역할을 구분해서 말할 수 있다.

발표용 문장:

```text
이번 프로젝트는 SQL 처리기를 새로 만드는 것이 아니라, 기존 DB 엔진을 HTTP API 서버로 외부에 열어주는 작업입니다.
```

## 3단계. 요청 흐름 코드 추적하기

목표: 클라이언트 요청이 코드에서 어떤 함수들을 거치는지 따라간다.

읽을 순서:

- [ ] `server_main.c`
- [ ] `server/server.c`
- [ ] `server/thread_pool.c`
- [ ] `server/http.c`
- [ ] `server/api.c`
- [ ] `sql_processor/sql.c`

외울 흐름:

```text
client
-> accept
-> thread_pool_submit
-> worker thread
-> http_read_request
-> api_handle_query
-> sql_execute
-> SQLResult
-> JSON response
```

### `server_main.c`

- [ ] 기본 포트가 `8080`인지 확인한다.
- [ ] `worker_count = 4`인지 확인한다.
- [ ] `queue_capacity = 16`인지 확인한다.
- [ ] `backlog = 32`인지 확인한다.
- [ ] `SIGINT`, `SIGTERM` signal handler를 확인한다.

설명 문장:

```text
server_main.c는 서버 설정값을 만들고 signal handler를 등록한 뒤 server_run()으로 accept loop를 시작합니다.
```

### `server/server.c`

- [ ] `socket`, `bind`, `listen` 흐름을 찾는다.
- [ ] `accept` loop를 찾는다.
- [ ] `thread_pool_submit`을 호출하는 위치를 찾는다.
- [ ] queue full일 때 503 응답을 보내는 위치를 찾는다.
- [ ] 서버가 하나의 공유 `Table`과 하나의 `db_mutex`를 갖는지 확인한다.

설명 문장:

```text
메인 스레드는 SQL을 직접 처리하지 않고, accept한 client fd를 thread pool queue에 넣습니다.
```

### `server/http.c`

- [ ] HTTP request line 파싱 위치를 찾는다.
- [ ] `POST`만 허용하는 조건을 찾는다.
- [ ] `/query`만 허용하는 조건을 찾는다.
- [ ] `Content-Length`를 읽는 코드를 찾는다.
- [ ] body를 SQL 문자열로 저장하는 코드를 찾는다.
- [ ] 405, 404, 413 오류 조건을 확인한다.

설명 문장:

```text
이 서버는 범용 HTTP 서버가 아니라 SQL API에 필요한 최소 HTTP parser만 구현했습니다.
```

### `server/api.c`

- [ ] `pthread_mutex_lock(db_mutex)` 위치를 찾는다.
- [ ] `sql_execute(table, sql)` 호출 위치를 찾는다.
- [ ] `SQLResult`를 JSON으로 바꾸는 함수를 찾는다.
- [ ] `pthread_mutex_unlock(db_mutex)` 위치를 찾는다.
- [ ] SQL 문법 오류가 JSON 오류 응답으로 내려가는 흐름을 확인한다.

설명 문장:

```text
SQL 엔진과 Table/B+Tree가 thread-safe하지 않기 때문에 SQL 실행과 결과 JSON 직렬화를 같은 mutex 안에서 처리합니다.
```

완료 기준:

- [ ] 요청 흐름을 코드 파일 이름과 함께 설명할 수 있다.
- [ ] `accept -> queue -> worker -> parser -> sql_execute -> JSON` 흐름을 막힘없이 말할 수 있다.

## 4단계. 동시성 구조 이해하기

목표: thread pool, queue, mutex, condition variable을 발표 수준으로 설명한다.

읽을 파일:

- [ ] `server/thread_pool.h`
- [ ] `server/thread_pool.c`
- [ ] `server/server.c`
- [ ] `server/api.c`

확인할 구현:

- [ ] worker thread를 미리 만드는 위치를 찾는다.
- [ ] worker가 queue가 비었을 때 `pthread_cond_wait`로 기다리는 위치를 찾는다.
- [ ] job queue의 `head`, `tail`, `size`가 circular queue로 움직이는지 확인한다.
- [ ] `thread_pool_submit`에서 queue가 full이면 실패하는지 확인한다.
- [ ] `stop_requested`와 `pthread_cond_broadcast`가 shutdown에 쓰이는지 확인한다.
- [ ] DB 접근은 별도의 `db_mutex`로 보호되는지 확인한다.

Q&A 준비:

- [ ] 왜 요청마다 새 thread를 만들지 않았는가?
- [ ] 왜 bounded queue가 필요한가?
- [ ] 왜 condition variable이 필요한가?
- [ ] 왜 DB mutex가 필요한가?
- [ ] 이 구조에서 어느 부분은 병렬이고 어느 부분은 직렬인가?

답변 메모:

```text
요청마다 thread를 만들면 생성/해제 비용이 크고 요청 폭주 시 thread 수가 무제한으로 늘어날 수 있습니다. 그래서 미리 worker를 4개 만들어두고 queue에 들어온 connection을 처리하게 했습니다.
```

```text
bounded queue는 서버가 감당할 수 있는 요청 수를 제한하기 위한 장치입니다. queue가 가득 차면 현재 구현은 503 Server is busy를 반환합니다.
```

```text
모든 요청이 하나의 in-memory Table을 공유하고, INSERT 시 next_id, row 배열, B+Tree가 변경됩니다. 동시에 접근하면 race condition이 생길 수 있어서 DB mutex로 보호합니다.
```

```text
네트워크 요청 파싱과 응답 처리는 여러 worker에서 병렬로 수행됩니다. 다만 실제 DB 실행 구간은 하나의 mutex로 보호되므로 직렬화됩니다. 안정성을 우선한 설계입니다.
```

완료 기준:

- [ ] thread pool 구조를 그림 없이 말로 설명할 수 있다.
- [ ] DB mutex가 없으면 어떤 문제가 생기는지 예시를 들 수 있다.
- [ ] queue full 시 503이 반환된다는 것을 설명할 수 있다.

## 5단계. DB 엔진 연결 이해하기

목표: 기존 SQL 처리기와 B+Tree가 API 서버와 어떻게 연결되는지 이해한다.

읽을 순서:

- [ ] `sql_processor/sql.h`
- [ ] `sql_processor/sql.c`
- [ ] `sql_processor/table.h`
- [ ] `sql_processor/table.c`
- [ ] `sql_processor/bptree.h`
- [ ] `sql_processor/bptree.c`

지원 SQL 확인:

- [ ] `INSERT INTO users VALUES ('Alice', 20);`
- [ ] `SELECT * FROM users;`
- [ ] `SELECT * FROM users WHERE id = 1;`
- [ ] `SELECT * FROM users WHERE id >= 10;`
- [ ] `SELECT * FROM users WHERE name = 'Alice';`
- [ ] `SELECT * FROM users WHERE age > 20;`

확인할 구현:

- [ ] `sql_execute(Table *table, const char *input)` 함수 위치를 찾는다.
- [ ] `SQLResult` 구조체 필드를 확인한다.
- [ ] `Table` 구조체에 `next_id`, `rows`, `pk_index`가 있는지 확인한다.
- [ ] `table_insert`에서 auto increment id가 부여되는지 확인한다.
- [ ] `table_insert`에서 B+Tree에 id와 record pointer를 넣는지 확인한다.
- [ ] `table_find_by_id_condition`이 B+Tree leaf를 사용하는지 확인한다.
- [ ] `table_find_by_name_matches`가 선형 탐색인지 확인한다.
- [ ] `table_find_by_age_condition`이 선형 탐색인지 확인한다.

설명 문장:

```text
기존 SQL 처리기는 sql_execute(Table *table, const char *input) 하나로 감싸져 있어서, API 서버는 SQL parser를 새로 만들지 않고 이 함수를 호출하는 방식으로 재사용했습니다.
```

```text
id는 primary key처럼 증가하는 값이고 B+Tree에 key-value 형태로 저장됩니다. 그래서 WHERE id 조건은 B+Tree index를 따라가며 찾습니다. 반면 name, age에는 별도 index가 없어서 row 배열을 처음부터 순회합니다.
```

완료 기준:

- [ ] `id` 조건 조회와 `name/age` 조건 조회의 차이를 설명할 수 있다.
- [ ] `SQLResult`가 API JSON 응답으로 바뀌는 흐름을 설명할 수 있다.
- [ ] 현재 DB가 in-memory only라는 한계를 설명할 수 있다.

## 6단계. 테스트와 데모 준비하기

목표: 발표 중 터미널 데모를 안정적으로 수행한다.

### 빌드 검증

- [ ] 루트에서 서버 빌드를 검증한다.

```bash
make clean && make db_server
```

### SQL 단위 테스트

- [ ] SQL 처리기 단위 테스트를 검증한다.

```bash
cd sql_processor
make clean && make unit_test && ./unit_test
cd ..
```

기대 출력:

```text
All unit tests passed.
```

### 서버 실행

- [ ] 서버를 실행한다.

```bash
./db_server 8080
```

### smoke test

- [ ] 다른 터미널에서 smoke test를 실행한다.

```bash
sh scripts/smoke_test.sh 8080
```

### 조건 조회 데모

- [ ] `WHERE id >= 1` 조건 조회를 실행한다.

```bash
curl -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "SELECT * FROM users WHERE id >= 1;"
```

### 동시 요청 데모

- [ ] 20개 INSERT를 병렬로 보낸다.

```bash
seq 1 20 | xargs -n1 -P8 -I{} curl -s -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "INSERT INTO users VALUES ('User{}', {});"
```

- [ ] 전체 데이터를 다시 조회한다.

```bash
curl -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "SELECT * FROM users;"
```

주의:

- [ ] 동시 INSERT 응답 순서는 입력 순서와 다를 수 있음을 알고 있다.
- [ ] 중요한 검증 기준은 `inserted_id`가 중복되지 않고 최종 SELECT에서 데이터가 유지되는 것이다.

### 오류 응답 데모

- [ ] 잘못된 SQL이 서버 crash가 아니라 JSON 오류로 반환되는지 확인한다.

```bash
curl -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "SELECT * FORM users;"
```

- [ ] GET 요청이 405로 반환되는지 확인한다.

```bash
curl -i -X GET http://localhost:8080/query
```

- [ ] 잘못된 path가 404로 반환되는지 확인한다.

```bash
curl -i -X POST http://localhost:8080/bad \
  -H "Content-Type: text/plain" \
  --data "SELECT * FROM users;"
```

완료 기준:

- [ ] 발표 중 사용할 명령을 직접 실행해봤다.
- [ ] 정상 INSERT, SELECT, 조건 조회, 동시 INSERT를 보여줄 수 있다.
- [ ] 오류 SQL이 JSON 오류로 반환되는 것을 보여줄 수 있다.

## 7단계. 4분 발표 스크립트 암기하기

목표: 제한 시간 안에 구조, 흐름, 동시성, 데모, 한계를 말한다.

### 0:00~0:30. 과제 목표와 한 문장 소개

- [ ] 아래 문장을 자연스럽게 말할 수 있다.

```text
저희 프로젝트는 기존 REPL 기반 SQL 엔진을 HTTP API 서버로 감싼 미니 DBMS입니다. 클라이언트는 POST /query로 SQL 문자열을 보내고, 서버는 기존 SQL 처리기와 B+Tree 기반 Table에 실행한 뒤 JSON으로 응답합니다.
```

### 0:30~1:10. 전체 아키텍처

- [ ] API 서버, thread pool, DB 엔진의 역할을 구분해서 말한다.

```text
구조는 세 계층입니다. 첫 번째는 TCP/HTTP 요청을 받는 API 서버, 두 번째는 connection을 worker에게 나눠주는 thread pool, 세 번째는 기존 SQL 처리기와 B+Tree를 포함한 DB 엔진입니다.
```

### 1:10~2:00. 요청 처리 흐름

- [ ] 아래 흐름을 말한다.

```text
요청 흐름은 accept -> queue -> worker -> HTTP parser -> api_handle_query -> sql_execute -> JSON response입니다. 메인 스레드는 connection을 accept하고 queue에 넣고, worker thread가 실제 요청을 읽고 SQL을 실행합니다.
```

### 2:00~2:45. 동시성 설계

- [ ] thread pool, bounded queue, DB mutex를 말한다.

```text
동시성은 thread pool과 DB mutex로 처리했습니다. worker는 여러 개라 요청 처리는 병렬로 받을 수 있지만, 공유 Table은 thread-safe하지 않기 때문에 SQL 실행 구간은 mutex로 보호했습니다. queue는 bounded queue라 과부하 시 503을 반환할 수 있습니다.
```

### 2:45~3:30. 데모

- [ ] INSERT 데모를 한다.
- [ ] SELECT 데모를 한다.
- [ ] `WHERE id >= 1` 조건 조회 데모를 한다.
- [ ] 시간이 있으면 동시 INSERT 데모를 한다.

### 3:30~4:00. 테스트 결과와 개선 방향

- [ ] 테스트 결과를 말한다.
- [ ] 현재 한계를 말한다.
- [ ] 개선 방향을 말한다.

```text
SQL 엔진은 unit test로 검증했고, API 서버는 smoke test와 curl 요청으로 검증했습니다. 동시 INSERT 요청에서도 ID가 중복되지 않고 서버가 유지되는 것을 확인했습니다.
```

```text
현재 한계는 in-memory only, 단일 users table, 제한된 SQL 문법, 전역 DB mutex입니다. 개선한다면 rwlock으로 SELECT 병렬성을 높이고, table을 여러 개 지원하며, persistence를 추가할 수 있습니다.
```

완료 기준:

- [ ] 4분 안에 발표를 끝낼 수 있다.
- [ ] 데모 명령을 보면서도 흐름 설명이 끊기지 않는다.
- [ ] 한계와 개선 방향을 스스로 말할 수 있다.

## 8단계. 팀원별 Q&A 백업 나누기

목표: 대표 발표자가 막혔을 때 팀원이 각 영역을 보충한다.

### 대표 발표자

- [ ] 전체 구조를 설명한다.
- [ ] 요청 흐름을 설명한다.
- [ ] 터미널 데모를 진행한다.

### API/HTTP 백업

- [ ] `POST /query` 계약을 설명한다.
- [ ] `Content-Length`가 필요한 이유를 설명한다.
- [ ] 404, 405, 413, 503 응답을 설명한다.
- [ ] SQL 오류와 HTTP 오류의 차이를 설명한다.

### 동시성 백업

- [ ] thread pool을 설명한다.
- [ ] circular queue를 설명한다.
- [ ] mutex와 condition variable을 설명한다.
- [ ] DB mutex의 필요성을 설명한다.
- [ ] queue full 시 503을 설명한다.

### DB 백업

- [ ] `sql_execute`를 설명한다.
- [ ] `Table` 구조를 설명한다.
- [ ] B+Tree index를 설명한다.
- [ ] `id` 조회와 `name/age` 조회의 차이를 설명한다.

### 테스트 백업

- [ ] `unit_test` 결과를 설명한다.
- [ ] `smoke_test.sh`가 무엇을 검증하는지 설명한다.
- [ ] 동시 INSERT 테스트를 설명한다.
- [ ] 오류 SQL 테스트를 설명한다.

완료 기준:

- [ ] 팀원 각자가 맡은 영역의 질문 2개 이상에 답할 수 있다.

## 최종 리허설 체크

- [ ] 발표자가 4분 타이머를 켜고 한 번 완주했다.
- [ ] 데모 명령을 순서대로 실행해봤다.
- [ ] 서버가 이미 켜져 있는지 확인하는 방법을 알고 있다.
- [ ] 포트 8080 충돌 시 다른 포트로 실행할 수 있다.
- [ ] 동시 INSERT 응답 순서가 섞여도 당황하지 않는다.
- [ ] `Ctrl+C`로 서버를 종료할 수 있다.
- [ ] Q&A 백업 담당자가 자기 영역을 숙지했다.

## 마지막으로 외울 5개 질문

- [ ] 이 프로젝트를 한 문장으로 설명하면?
- [ ] 요청 하나가 들어오면 코드에서 어떤 순서로 처리되는가?
- [ ] 왜 thread pool을 썼는가?
- [ ] 왜 DB mutex가 필요한가?
- [ ] `WHERE id >= 1`은 왜 B+Tree를 쓰고, `name/age`는 왜 선형 탐색인가?
