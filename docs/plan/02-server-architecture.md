# 02. Server Architecture

## 코드 주석 원칙

서버 아키텍처 코드를 작성할 때는 accept loop, job queue 전달, worker 처리, HTTP 파싱, DB 호출, 응답 반환의 경계마다 한국어 주석을 친절하게 남깁니다. 각 함수가 서버 흐름에서 맡는 역할과 다음 계층으로 무엇을 넘기는지 설명할 수 있어야 합니다.

이 문서는 C 기반 API 서버의 전체 구조를 정의합니다.

## 전체 흐름

```text
client
  -> TCP connection
  -> HTTP request
  -> server accept loop
  -> job queue
  -> worker thread
  -> HTTP parser
  -> sql_execute(shared_table, sql)
  -> SQLResult serialization
  -> HTTP response
  -> close connection
```

## 주요 컴포넌트

### Server

- listening socket을 생성합니다.
- 지정된 port에 bind하고 listen합니다.
- accept loop에서 client connection fd를 받습니다.
- connection fd를 직접 처리하지 않고 job queue에 넣습니다.

### Job Queue

- worker thread가 가져갈 client fd를 저장합니다.
- queue는 mutex로 보호합니다.
- queue가 비어 있으면 worker는 condition variable에서 대기합니다.
- queue가 가득 찬 경우 accept loop는 503 응답 후 연결을 닫거나, 여유가 생길 때까지 잠시 대기합니다. MVP는 503 응답을 기본값으로 둡니다.

### Worker Thread

- 서버 시작 시 고정 개수로 생성됩니다.
- queue에서 client fd를 하나 꺼냅니다.
- HTTP 요청을 읽고 SQL 문자열을 추출합니다.
- 공유 DB 엔진에 SQL을 실행합니다.
- HTTP 응답을 쓰고 connection을 닫습니다.

### DB Context

- 서버 전체에서 하나의 `Table *shared_table`을 유지합니다.
- `sql_execute(shared_table, sql)` 호출을 통해 기존 엔진을 사용합니다.
- shared table과 B+Tree는 thread-safe하지 않으므로 DB mutex로 보호합니다.

## 요청 처리 책임 분리

- socket 계층: 연결 생성, accept, read, write, close.
- HTTP 계층: method, path, body 추출, status code 결정.
- API 계층: `/query` 라우팅, SQL 문자열 검증.
- DB 계층: `sql_execute()` 호출과 `SQLResult` 정리.
- 응답 계층: SQL 결과를 JSON 또는 text 응답으로 변환.

## 실패 처리 원칙

- 잘못된 HTTP 요청은 `400 Bad Request`로 응답합니다.
- 지원하지 않는 path는 `404 Not Found`로 응답합니다.
- 지원하지 않는 method는 `405 Method Not Allowed`로 응답합니다.
- SQL 문법 오류 또는 query 오류는 HTTP `200 OK` 안에 SQL 오류 내용을 담거나, API 오류로 `400 Bad Request`를 반환합니다. MVP 기본값은 SQL 오류를 JSON body에 담아 반환하는 방식입니다.
- 서버 내부 메모리 할당 실패는 `500 Internal Server Error`로 응답합니다.
- 작업 큐가 가득 차면 `503 Service Unavailable`을 반환합니다.

## 구현 파일 방향

서버 구현 시 파일은 역할 기준으로 나누는 것을 권장합니다.

- `server.c`, `server.h`: listen socket, accept loop, server lifecycle.
- `thread_pool.c`, `thread_pool.h`: worker 생성, queue, shutdown.
- `http.c`, `http.h`: HTTP 요청 읽기와 응답 작성.
- `api.c`, `api.h`: `/query` 라우팅과 SQL 응답 직렬화.
- 기존 `sql.c`, `table.c`, `bptree.c`: 내부 DB 엔진으로 유지.
