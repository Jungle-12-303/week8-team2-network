# HTTP Smoke Test and Manual Check

이 문서는 현재 구현되어 있는 HTTP 확인 방법을 설명합니다.

## 무엇을 테스트하나

현재 서버는 `POST /query` 하나를 제공합니다.

`scripts/smoke_test.sh`는 이 흐름을 확인합니다.

- `INSERT INTO users VALUES ('Alice', 20);`
- `SELECT * FROM users;`

즉, 이 테스트는 다음을 확인합니다.

- 서버가 실제로 뜨는지
- `POST /query`가 동작하는지
- `INSERT`가 성공하는지
- `SELECT`가 이어서 동작하는지
- HTTP 응답이 JSON으로 내려오는지

## 수동 실행 전 준비

서버를 먼저 실행해야 합니다.

### Docker로 실행

이거 추천함요
```bash
docker compose up --build
```

또는

```bash
docker build -t week8-team2-network .
docker run --rm -p 8080:8080 week8-team2-network
```



실행 후 로그에 아래 문구가 보이면 됩니다.

```text
Listening on port 8080
```

## 스모크 테스트 실행

서버가 켜진 상태에서 프로젝트 루트에서 실행합니다.

```bash
sh scripts/smoke_test.sh
```

## 기대 결과

현재 스크립트는 응답 본문을 확인하고, `ok`, `action`, `Alice`가 없으면 실패합니다.
출력은 `INSERT`, `SELECT` 구분선과 함께 줄바꿈된 JSON 형태로 보여 줍니다.

실행하면 요청별 응답 본문도 그대로 출력합니다.

- 첫 번째 요청이 `INSERT` 성공 응답처럼 보인다
- 두 번째 요청이 `SELECT` 성공 응답처럼 보인다

## 수동으로 직접 확인하는 방법

### 1) INSERT 확인

```bash
curl -v -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "INSERT INTO users VALUES ('Alice', 20);"
```

확인 포인트:

- HTTP `200`
- JSON 응답
- `ok:true`
- `action:"insert"`

### 2) SELECT 확인

```bash
curl -v -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "SELECT * FROM users;"
```

확인 포인트:

- HTTP `200`
- JSON 응답
- `ok:true`
- `action:"select"`
- `rows` 안에 `Alice`가 보이는지 확인

### 3) 잘못된 메서드 확인

```bash
curl -v -X GET http://localhost:8080/query
```

확인 포인트:

- HTTP `405`
- 에러 JSON

### 4) 잘못된 경로 확인

```bash
curl -v -X POST http://localhost:8080/unknown \
  -H "Content-Type: text/plain" \
  --data "SELECT * FROM users;"
```

확인 포인트:

- HTTP `404`

## 같이 보면 좋은 파일

- [scripts/smoke_test.sh](../../../scripts/smoke_test.sh)
- [scripts/manual_query.sh](../../../scripts/manual_query.sh)
- [docs/test/demo/manual-query.md](manual-query.md)
- [scripts/http_integration_test.sh](../../../scripts/http_integration_test.sh)
- [server/server.c](../../../server/server.c)
- [server/http.c](../../../server/http.c)
- [server/api.c](../../../server/api.c)

