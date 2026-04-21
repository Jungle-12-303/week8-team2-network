# 07. Test and Quality Plan

## 코드 주석 원칙

테스트 코드에도 한국어 주석을 친절하게 남깁니다. 테스트가 어떤 요구사항을 검증하는지, 어떤 엣지 케이스를 재현하는지, 동시성 테스트가 어떤 실패를 잡기 위한 것인지 짧게 설명합니다.

이 문서는 단위 테스트, API 기능 테스트, 동시성 테스트, 발표 전 검증 기준을 정리합니다.

## 기존 단위 테스트

기존 SQL 처리기 테스트는 서버 구현 후에도 계속 통과해야 합니다.

```bash
cd sql_processor
make
./unit_test
```

확인할 항목:

- B+Tree insert/search.
- duplicate key 거부.
- leaf split과 internal split 이후 검색 유지.
- leaf link 기반 range query.
- `INSERT`, `SELECT`, `WHERE id/name/age` 처리.
- SQL 오류 메시지.

## API 기능 테스트

서버 실행 후 curl로 최소 다음 요청을 검증합니다.

```bash
curl -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "INSERT INTO users VALUES ('Alice', 20);"
```

```bash
curl -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "SELECT * FROM users;"
```

```bash
curl -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "SELECT * FROM users WHERE id >= 1;"
```

검증 기준:

- HTTP status가 기대와 일치합니다.
- JSON body가 깨지지 않습니다.
- INSERT 후 SELECT에서 row가 조회됩니다.
- SQL 문법 오류가 서버 crash 없이 응답으로 반환됩니다.

## HTTP 엣지 케이스

- `GET /query`는 `405 Method Not Allowed`.
- `POST /unknown`은 `404 Not Found`.
- body가 없는 `POST /query`는 `400 Bad Request`.
- `Content-Length`가 너무 크면 `413 Payload Too Large`.
- malformed request line은 `400 Bad Request`.
- 요청이 중간에 끊긴 경우도 `400 Bad Request`로 확인합니다.
- `Content-Length`와 실제 body 길이가 맞지 않는 경우를 별도로 검증합니다.

## 동시성 테스트

동시에 여러 요청을 보내 스레드 풀이 동작하는지 확인합니다.

```bash
seq 1 20 | xargs -n1 -P8 -I{} curl -s -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "INSERT INTO users VALUES ('User{}', {});"
```

검증 기준:

- 서버가 죽지 않습니다.
- 응답이 모두 반환됩니다.
- 이후 `SELECT * FROM users;`에서 insert된 row들이 조회됩니다.
- id가 중복되거나 깨지지 않습니다.
- queue full 상황에서 `503`이 나오는지 확인합니다.
- 종료 신호 이후 새 요청을 받지 않는지 확인합니다.

## 품질 기준

- warning 없이 빌드합니다.
- 메모리 할당 실패 경로를 최소한 오류 응답으로 처리합니다.
- socket fd는 모든 경로에서 닫습니다.
- `SQLResult`는 모든 경로에서 정리합니다.
- worker thread가 queue mutex를 잡은 채로 오래 걸리는 작업을 하지 않습니다.
- JSON 직렬화 실패 경로도 확인합니다.
- graceful shutdown 시 hang이 없는지 확인합니다.

## 발표 전 체크리스트

- README의 빌드 명령어가 실제로 동작합니다.
- README의 curl 예시가 실제로 동작합니다.
- 단위 테스트 결과를 보여줄 수 있습니다.
- API 기능 테스트 결과를 보여줄 수 있습니다.
- 스레드 풀 구조를 그림 또는 텍스트 흐름으로 설명할 수 있습니다.
- 공유 DB lock을 왜 넣었는지 설명할 수 있습니다.
