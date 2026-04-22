# 3단계. 요청 흐름 코드 추적하기

## 목표

클라이언트 요청 하나가 코드 안에서 어떤 함수와 모듈을 거쳐 처리되는지 추적합니다. 발표의 중심은 이 단계입니다.

## 예상 소요 시간

- 60~70분

## 최종적으로 외울 흐름

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

## 읽을 파일 순서

- [ ] `server_main.c`
- [ ] `server/server.c`
- [ ] `server/thread_pool.c`
- [ ] `server/http.c`
- [ ] `server/api.c`
- [ ] `sql_processor/sql.c`

## 3-1. `server_main.c`에서 시작점 찾기

확인할 코드:

- [ ] `main` 함수
- [ ] 포트 인자 파싱
- [ ] signal handler 설치
- [ ] `ServerConfig` 설정
- [ ] `server_create`
- [ ] `server_run`
- [ ] `server_destroy`

체크 포인트:

- [ ] 기본 포트는 `8080`이다.
- [ ] `worker_count`는 `4`다.
- [ ] `queue_capacity`는 `16`이다.
- [ ] `backlog`는 `32`다.
- [ ] `SIGINT`, `SIGTERM`을 처리한다.

코드 읽기 질문:

- [ ] 포트가 잘못 들어오면 어떻게 되는가?
- [ ] signal handler는 어떤 함수를 호출하는가?
- [ ] 서버 설정값은 어디에서 정해지는가?

설명 문장:

```text
server_main.c는 CLI 인자로 포트를 받고, signal handler를 등록한 뒤, worker 수와 queue 크기 같은 서버 설정을 만들어 server_run()을 호출합니다.
```

## 3-2. `server_create`에서 서버 객체 구성 보기

파일:

- [ ] `server/server.c`

확인할 코드:

- [ ] `struct Server`
- [ ] `table_create`
- [ ] `pthread_mutex_init(&server->db_mutex, NULL)`
- [ ] `thread_pool_init`
- [ ] `server_make_listen_socket`

`struct Server`에서 확인할 필드:

- [ ] `listen_fd`
- [ ] `Table *table`
- [ ] `pthread_mutex_t db_mutex`
- [ ] `ThreadPool pool`
- [ ] `ServerConfig config`
- [ ] `initialized`

체크 질문:

- [ ] 서버 안에 `Table`은 몇 개 만들어지는가?
- [ ] 모든 worker가 같은 `Table`을 공유하는가?
- [ ] DB mutex는 왜 `Server` 구조체 안에 있는가?

답변 힌트:

```text
서버 프로세스 안에 하나의 shared Table이 있고, 모든 요청이 이 Table을 공유합니다. 따라서 shared Table을 보호하기 위한 db_mutex도 Server가 소유합니다.
```

## 3-3. listen socket 생성 흐름 보기

파일:

- [ ] `server/server.c`

확인할 함수:

- [ ] `server_make_listen_socket`

확인할 시스템 콜:

- [ ] `socket`
- [ ] `setsockopt`
- [ ] `bind`
- [ ] `listen`

체크 질문:

- [ ] `socket(AF_INET, SOCK_STREAM, 0)`은 어떤 종류의 socket인가?
- [ ] `SO_REUSEADDR`는 왜 설정하는가?
- [ ] `bind`는 무엇을 하는가?
- [ ] `listen`의 backlog는 무엇인가?

답변 힌트:

```text
TCP IPv4 socket을 만들고, 특정 포트에 bind한 뒤, listen 상태로 만들어 클라이언트 연결을 받을 준비를 합니다.
```

## 3-4. accept loop 따라가기

파일:

- [ ] `server/server.c`

확인할 함수:

- [ ] `server_run`

확인할 코드:

- [ ] `while (!server_shutdown_requested())`
- [ ] `accept`
- [ ] `thread_pool_submit`
- [ ] queue full일 때 503 응답
- [ ] `thread_pool_shutdown`

흐름 정리:

```text
accept로 client_fd 받기
-> thread_pool_submit에 client_fd 넘기기
-> submit 실패 시 503 응답 후 close
-> shutdown 요청 시 loop 종료
```

체크 질문:

- [ ] 메인 스레드가 직접 HTTP 요청을 읽는가?
- [ ] 메인 스레드가 직접 SQL을 실행하는가?
- [ ] queue가 가득 차면 어떤 응답을 보내는가?

답변 힌트:

```text
메인 스레드는 client fd를 accept해서 thread pool queue에 넣는 역할만 합니다. 실제 HTTP parsing과 SQL 실행은 worker가 처리합니다.
```

## 3-5. worker가 client fd를 처리하는 흐름 보기

파일:

- [ ] `server/thread_pool.c`
- [ ] `server/server.c`

확인할 함수:

- [ ] `thread_pool_worker_main`
- [ ] `server_handle_client`

worker 흐름:

```text
queue에서 client_fd 꺼내기
-> handler 호출
-> handler가 server_handle_client 실행
-> 처리 후 close(client_fd)
```

`server_handle_client` 내부 흐름:

- [ ] `http_read_request`
- [ ] 실패 시 HTTP 오류 JSON 응답
- [ ] 성공 시 `api_handle_query`
- [ ] API 결과를 `http_send_response`로 전송
- [ ] `api_result_destroy`

체크 질문:

- [ ] worker는 SQL 문자열을 어디에서 얻는가?
- [ ] HTTP 파싱 실패와 SQL 실행 실패는 같은 흐름인가?
- [ ] client fd는 어디에서 close되는가?

답변 힌트:

```text
worker는 server_handle_client를 호출하고, server_handle_client 안에서 http_read_request가 body를 SQL 문자열로 읽습니다.
```

## 3-6. HTTP request parser 흐름 보기

파일:

- [ ] `server/http.c`
- [ ] `server/http.h`

확인할 함수:

- [ ] `http_read_request`
- [ ] `read_until_header_end`
- [ ] `http_send_response`

확인할 상수:

- [ ] `HTTP_MAX_BODY_SIZE`
- [ ] `HTTP_MAX_REQUEST_SIZE`

`http_read_request` 흐름:

- [ ] header 끝인 `\r\n\r\n`을 찾는다.
- [ ] request line에서 method, path, version을 읽는다.
- [ ] method가 `POST`인지 확인한다.
- [ ] path가 `/query`인지 확인한다.
- [ ] header에서 `Content-Length`를 찾는다.
- [ ] body 크기가 제한을 넘지 않는지 확인한다.
- [ ] body를 끝까지 읽는다.
- [ ] `request->body`에 null terminator를 붙인다.

체크 질문:

- [ ] `Content-Length`가 없으면 어떻게 되는가?
- [ ] body가 너무 크면 어떻게 되는가?
- [ ] 왜 `request->body[content_length] = '\0'`가 필요한가?

답변 힌트:

```text
SQL 엔진은 C 문자열을 입력으로 받기 때문에 body 뒤에 null terminator가 필요합니다.
```

## 3-7. API adapter 흐름 보기

파일:

- [ ] `server/api.c`
- [ ] `server/api.h`

확인할 함수:

- [ ] `api_handle_query`
- [ ] `api_build_success_response`
- [ ] `api_build_error_response`
- [ ] `api_append_record`

`api_handle_query` 흐름:

```text
db_mutex lock
-> sql_execute(table, sql)
-> SQLResult를 JSON으로 변환
-> sql_result_destroy
-> db_mutex unlock
-> ApiResult 반환
```

HTTP status 정리:

- [ ] SQL 성공: 200
- [ ] SQL not found: 200, `rows: []`
- [ ] SQL syntax/query error: 200, `ok:false`
- [ ] EXIT/QUIT: 400
- [ ] 내부 DB 오류: 500

체크 질문:

- [ ] SQL 오류인데 왜 HTTP 200을 반환하는 경우가 있는가?
- [ ] JSON 직렬화를 mutex 안에서 하는 이유는 무엇인가?
- [ ] `sql_result_destroy`는 왜 필요한가?

답변 힌트:

```text
SQLResult 안의 records는 Table이 소유한 Record pointer를 가리킵니다. unlock 후 다른 요청이 Table을 바꾸는 상황을 피하려고 현재 구현은 SQL 실행과 JSON 직렬화를 같은 lock 안에서 처리합니다.
```

## 3-8. SQL 엔진 진입점 확인

파일:

- [ ] `sql_processor/sql.c`
- [ ] `sql_processor/sql.h`

확인할 함수:

- [ ] `sql_execute`
- [ ] `sql_execute_insert`
- [ ] `sql_execute_select`
- [ ] `sql_result_destroy`

`sql_execute` 흐름:

- [ ] 입력 앞 공백을 건너뛴다.
- [ ] `EXIT`, `QUIT`를 먼저 검사한다.
- [ ] INSERT 문법을 시도한다.
- [ ] SELECT 문법을 시도한다.
- [ ] 실패하면 syntax error를 만든다.

체크 질문:

- [ ] API 서버는 SQL 문법을 직접 파싱하는가?
- [ ] SQL parser는 어느 모듈에 있는가?
- [ ] `sql_execute`의 입력과 출력 타입은 무엇인가?

답변 힌트:

```text
API 서버는 SQL parser를 직접 구현하지 않고, 기존 sql_processor의 sql_execute(Table *, const char *)를 호출합니다.
```

## 3-9. 요청 흐름 직접 말하기 훈련

아래 문장을 보지 않고 말해봅니다.

```text
클라이언트가 POST /query로 SQL 문자열을 보내면, 메인 스레드가 accept로 connection fd를 받고 thread_pool_submit으로 queue에 넣습니다. worker thread는 queue에서 fd를 꺼내 HTTP 요청을 읽고, body에서 SQL 문자열을 얻습니다. 그 다음 api_handle_query가 DB mutex를 잡고 sql_execute를 호출합니다. sql_execute 결과인 SQLResult는 JSON으로 변환되고, http_send_response로 클라이언트에게 돌아갑니다.
```

검증 질문:

- [ ] `accept`는 어느 파일에 있는가?
- [ ] `thread_pool_submit`은 어느 파일에 있는가?
- [ ] `http_read_request`는 어느 파일에 있는가?
- [ ] `api_handle_query`는 어느 파일에 있는가?
- [ ] `sql_execute`는 어느 파일에 있는가?

## 발표용 핵심 문장

```text
요청 흐름은 accept -> queue -> worker -> HTTP parser -> api_handle_query -> sql_execute -> JSON response입니다.
```

```text
메인 스레드는 connection을 accept하고 queue에 넣고, worker thread가 실제 요청을 읽고 SQL을 실행합니다.
```

```text
API 서버는 DB 로직을 직접 구현하지 않고, 기존 sql_execute 함수를 호출하는 adapter 역할을 합니다.
```

## 완료 기준

- [ ] `server_main.c`에서 서버 시작 흐름을 설명할 수 있다.
- [ ] `server/server.c`에서 accept loop를 설명할 수 있다.
- [ ] `server/thread_pool.c`에서 worker 호출 흐름을 설명할 수 있다.
- [ ] `server/http.c`에서 SQL body를 읽는 흐름을 설명할 수 있다.
- [ ] `server/api.c`에서 SQL 실행과 JSON 변환 흐름을 설명할 수 있다.
- [ ] `sql_processor/sql.c`에서 SQL 엔진 진입점을 설명할 수 있다.
- [ ] 전체 요청 흐름을 파일 이름과 함께 말할 수 있다.
