# 1단계. 실행 결과 먼저 확인하기

## 목표

코드를 읽기 전에 이 프로젝트가 사용자 입장에서 어떻게 동작하는지 직접 확인합니다. 이 단계의 핵심은 “HTTP API로 SQL을 보내고 JSON을 받는다”를 몸으로 익히는 것입니다.

## 예상 소요 시간

- 20~30분

## 이 단계에서 얻어야 하는 것

- [ ] 서버를 빌드할 수 있다.
- [ ] 서버를 실행할 수 있다.
- [ ] `POST /query`로 SQL을 보낼 수 있다.
- [ ] INSERT 응답 JSON을 읽을 수 있다.
- [ ] SELECT 응답 JSON을 읽을 수 있다.
- [ ] “외부 클라이언트가 DBMS 기능을 HTTP로 사용한다”는 의미를 설명할 수 있다.

## 1-1. 프로젝트 루트 확인

- [ ] 현재 위치를 확인한다.

```bash
pwd
```

기대 위치:

```text
/Users/juhoseok/Desktop/week8-team2-network
```

- [ ] 위치가 다르면 프로젝트 루트로 이동한다.

```bash
cd /Users/juhoseok/Desktop/week8-team2-network
```

체크 질문:

- [ ] 왜 명령을 프로젝트 루트에서 실행해야 하는가?
  - 가장 최상단의 폴더에서 실행하는것이 편하니까 편하다는 의미는 폴더를 계속 타고 들어가지않고 루트에 실행파일을 만들어 놓아 폴더를 타고타고 들어가는 불상사를 막기위해  
- [ ] `Makefile`, `server_main.c`, `scripts/smoke_test.sh`가 어디에 있는가?
  - 프로젝트 최상단 
## 1-2. 서버 빌드

- [ ] 기존 빌드 산출물을 지우고 서버를 다시 빌드한다.
uhoseok@juhoseog-ui-MacBookAir week8-team2-network % make clean && make db_server
rm -f server_main.o server/json_util.o server/http.o server/thread_pool.o server/api.o server/server.o sql_processor/bptree.o sql_processor/table.o sql_processor/sql.o db_server
cc -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Werror -pedantic -O2 -Iserver -Isql_processor -c server_main.c -o server_main.o
cc -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Werror -pedantic -O2 -Iserver -Isql_processor -c server/json_util.c -o server/json_util.o
cc -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Werror -pedantic -O2 -Iserver -Isql_processor -c server/http.c -o server/http.o
cc -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Werror -pedantic -O2 -Iserver -Isql_processor -c server/thread_pool.c -o server/thread_pool.o
cc -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Werror -pedantic -O2 -Iserver -Isql_processor -c server/api.c -o server/api.o
cc -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Werror -pedantic -O2 -Iserver -Isql_processor -c server/server.c -o server/server.o
cc -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Werror -pedantic -O2 -Iserver -Isql_processor -c sql_processor/bptree.c -o sql_processor/bptree.o
cc -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Werror -pedantic -O2 -Iserver -Isql_processor -c sql_processor/table.c -o sql_processor/table.o
cc -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Werror -pedantic -O2 -Iserver -Isql_processor -c sql_processor/sql.c -o sql_processor/sql.o
cc -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Werror -pedantic -O2 -Iserver -Isql_processor -o db_server server_main.o server/json_util.o server/http.o server/thread_pool.o server/api.o server/server.o sql_processor/bptree.o sql_processor/table.o sql_processor/sql.o -pthread

```bash
make clean && make db_server
```

확인할 것:

- [ ] `db_server` 파일이 생성되었는가?
ㅇㅇ
- [ ] 컴파일 에러가 없는가?
ㅇㅇ
- [ ] `-pthread` 옵션으로 링크되는가?

체크 질문:

- [ ] 왜 `pthread`가 필요한가?
- [ ] 서버 빌드에 `server/*.c`뿐 아니라 `sql_processor/*.c`도 포함되는 이유는 무엇인가?

답변 힌트:

```text
API 서버가 기존 SQL 처리기와 B+Tree 구현을 함께 링크해서 하나의 실행 파일로 만들기 때문입니다.
```

## 1-3. 서버 실행

- [ ] 서버를 8080 포트로 실행한다.

```bash
./db_server 8080
```

기대 출력:

```text
Listening on port 8080
```

확인할 것:

- [ ] 서버가 종료되지 않고 대기 상태로 남아 있는가?
- [ ] 이 터미널은 서버 프로세스가 사용하므로 다른 터미널에서 curl을 실행해야 한다.

체크 질문:

- [ ] `./db_server 8080`에서 `8080`은 무엇인가?
- [ ] 포트를 생략하면 어떤 포트를 사용하는가?

답변 힌트:

```text
포트를 생략하면 server_main.c의 기본값인 8080을 사용합니다.
```

## 1-4. smoke test 실행

다른 터미널에서 실행합니다.

- [ ] smoke test를 실행한다.

```bash
sh scripts/smoke_test.sh 8080
```

확인할 것:

- [ ] 첫 번째 응답은 INSERT 결과인가?
- [ ] 두 번째 응답은 SELECT 결과인가?
- [ ] 응답이 JSON 문자열인가?

예상 응답 형태:

```json
{"ok":true,"action":"insert","inserted_id":1,"row_count":1}
{"ok":true,"action":"select","row_count":1,"rows":[{"id":1,"name":"Alice","age":20}]}
```

체크 질문:

- [ ] smoke test는 정확히 어떤 SQL 두 개를 보내는가?
- [ ] `scripts/smoke_test.sh`는 서버를 실행하는가, 아니면 이미 실행 중인 서버에 요청만 보내는가?

## 1-5. 직접 INSERT 요청 보내기

- [ ] curl로 INSERT 요청을 보낸다.

```bash
curl -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "INSERT INTO users VALUES ('Alice', 20);"
```

확인할 것:

- [ ] HTTP method는 `POST`인가?
- [ ] path는 `/query`인가?
- [ ] body는 SQL 문자열인가?
- [ ] 응답에 `ok`, `action`, `inserted_id`, `row_count`가 있는가?

응답 필드 의미:

- [ ] `ok`: SQL 실행 성공 여부
- [ ] `action`: SQL 종류, 여기서는 `insert`
- [ ] `inserted_id`: 새로 삽입된 row의 id
- [ ] `row_count`: 영향을 받은 row 수

체크 질문:

- [ ] 왜 SQL을 JSON body가 아니라 plain text body로 보냈는가?
- [ ] 이 프로젝트의 API 입력은 어떤 형식인가?

답변 힌트:

```text
현재 API 계약은 POST /query의 body 전체를 SQL 문자열로 해석하는 단순한 구조입니다.
```

## 1-6. 직접 SELECT 요청 보내기

- [ ] curl로 SELECT 요청을 보낸다.

```bash
curl -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "SELECT * FROM users;"
```

확인할 것:

- [ ] 응답의 `action`은 `select`인가?
- [ ] `rows` 배열이 있는가?
- [ ] 각 row에 `id`, `name`, `age`가 있는가?
- [ ] 앞에서 INSERT한 데이터가 유지되는가?

응답 필드 의미:

- [ ] `row_count`: 조회된 row 수
- [ ] `rows`: 조회 결과 배열
- [ ] `id`: auto increment id
- [ ] `name`: 사용자 이름
- [ ] `age`: 나이

체크 질문:

- [ ] 왜 INSERT 후 SELECT에서 데이터가 유지되는가?
- [ ] 서버를 재시작하면 데이터는 유지되는가?

답변 힌트:

```text
서버 프로세스 안의 하나의 in-memory Table에 데이터가 저장되기 때문에 서버가 살아 있는 동안 유지됩니다. persistence는 없으므로 재시작하면 사라집니다.
```

## 1-7. 조건 조회 맛보기

- [ ] id 조건 조회를 실행한다.

```bash
curl -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "SELECT * FROM users WHERE id >= 1;"
```

- [ ] name 조건 조회를 실행한다.

```bash
curl -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "SELECT * FROM users WHERE name = 'Alice';"
```

- [ ] age 조건 조회를 실행한다.

```bash
curl -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "SELECT * FROM users WHERE age > 10;"
```

체크 질문:

- [ ] 현재 지원되는 테이블 이름은 무엇인가?
- [ ] 현재 지원되는 컬럼은 무엇인가?
- [ ] 모든 SELECT 문법이 지원되는가, 제한된 문법만 지원되는가?

## 1-8. 오류 응답 맛보기

- [ ] 잘못된 SQL을 보내본다.

```bash
curl -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "SELECT * FORM users;"
```

확인할 것:

- [ ] 서버가 crash하지 않는가?
- [ ] `ok:false` JSON이 내려오는가?
- [ ] `status`, `error_code`, `sql_state`, `message`가 있는가?

- [ ] 잘못된 HTTP method를 보내본다.

```bash
curl -i -X GET http://localhost:8080/query
```

확인할 것:

- [ ] HTTP status가 405인가?
- [ ] JSON error body가 내려오는가?

체크 질문:

- [ ] SQL 오류와 HTTP 오류는 어떻게 다른가?

답변 힌트:

```text
SQL 문법 오류는 API가 정상적으로 SQL 엔진까지 도달한 결과이고, GET /query 같은 오류는 HTTP 요청 자체가 API 계약과 맞지 않는 경우입니다.
```

## 1-9. 서버 종료

- [ ] 서버를 실행한 터미널에서 `Ctrl+C`로 종료한다.
- [ ] 서버가 정상 종료되는지 확인한다.

체크 질문:

- [ ] `Ctrl+C`는 어떤 signal을 보내는가?
- [ ] 이 signal은 어느 파일에서 처리되는가?

답변 힌트:

```text
Ctrl+C는 SIGINT를 보내고, server_main.c의 signal handler가 server_signal_shutdown()을 호출합니다.
```

## 발표용 핵심 문장

```text
클라이언트는 HTTP POST /query로 SQL 문자열을 보내고, 서버는 기존 SQL 엔진에 실행시킨 뒤 JSON으로 응답합니다.
```

```text
INSERT 결과는 inserted_id와 row_count로 확인하고, SELECT 결과는 rows 배열로 확인합니다.
```

```text
서버가 살아 있는 동안 데이터는 in-memory Table에 유지되지만, persistence는 아직 없습니다.
```

## 완료 기준

- [ ] `make clean && make db_server`를 성공했다.
- [ ] `./db_server 8080`으로 서버를 실행했다.
- [ ] `sh scripts/smoke_test.sh 8080`을 성공했다.
- [ ] 직접 INSERT curl을 실행했다.
- [ ] 직접 SELECT curl을 실행했다.
- [ ] 조건 조회 curl을 실행했다.
- [ ] 잘못된 SQL의 JSON 오류 응답을 확인했다.
- [ ] `POST /query`의 입력과 출력 형식을 설명할 수 있다.
- [ ] 서버가 in-memory 방식이라는 점을 설명할 수 있다.
