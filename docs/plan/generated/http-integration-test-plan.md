# HTTP Integration Test Plan

이 문서는 이 프로젝트의 HTTP 통합 테스트를 어떻게 더 추가할지 정리한 계획 문서입니다.

현재 실제로 구현되어 있는 테스트는 `sql_processor` 단위 테스트, `scripts/smoke_test.sh`, `scripts/http_integration_test.sh` 입니다.  
따라서 이 문서는 `현재 있는 테스트`와 `앞으로 추가할 테스트`를 분리해서 적습니다.

추가로, Docker 기반 HTTP 통합 테스트를 한 번에 돌릴 수 있는 스크립트와 `make` 타겟도 함께 제공한다.

현재 서버는 `POST /query` 하나를 제공하고, 요청 본문에 들어온 SQL을 `sql_processor`로 넘겨서 JSON 응답을 돌려줍니다.  
즉, 통합 테스트의 핵심은 다음 3가지를 같이 보는 것입니다.

1. HTTP 레벨에서 올바른 상태 코드가 나오는지
2. JSON 응답 구조가 기대와 맞는지
3. SQL 실행 결과가 실제 데이터 상태와 일치하는지

## 1. 지금 코드 기준으로 확인할 수 있는 경계

현재 구현을 기준으로 보면 검증 포인트가 꽤 명확합니다.

- `POST /query` 만 허용
- `Content-Length` 필수
- 최대 본문 크기 제한 존재
- 요청 라인/헤더가 깨지면 `400`
- 잘못된 메서드는 `405`
- 존재하지 않는 경로는 `404`
- 요청 본문이 너무 크면 `413`
- SQL 실행 성공 시 JSON 응답
- `SELECT` 결과가 없더라도 서버는 정상 응답을 반환
- `EXIT` / `QUIT` 은 HTTP로는 지원하지 않음
- 동시 요청이 몰리면 queue full 상황에서 `503` 가능

이런 이유로 HTTP 통합 테스트는 단순히 `curl` 한 번 치는 수준보다, 아래처럼 `정상 흐름 + 실패 흐름 + 경계값`을 함께 보는 방식이 좋습니다.

## 2. 현재 구현되어 있는 테스트

### 2-1. `sql_processor` 단위 테스트

- 위치: `sql_processor/unit_test.c`
- 실행: `cd sql_processor && make && ./unit_test`
- 역할: B+Tree, Table, SQL 실행 결과의 핵심 로직 검증

### 2-2. HTTP 스모크 테스트

- 위치: `scripts/smoke_test.sh`
- 실행: 서버 실행 후 `sh scripts/smoke_test.sh`
- 역할: `POST /query`의 기본적인 `INSERT` / `SELECT` 흐름 확인

### 2-3. HTTP 통합 테스트 스크립트

- 위치: `scripts/http_integration_test.sh`
- 실행: `sh scripts/http_integration_test.sh`
- 역할:
  - Docker 이미지를 빌드한다
  - 임시 컨테이너를 띄운다
  - `INSERT`, `SELECT`, `405`, `404`, `413` 을 확인한다
  - 종료 시 컨테이너를 정리한다

### 2-4. `make` 타겟

- `make test-unit`: `sql_processor` 단위 테스트 실행
- `make test-http`: Docker 기반 HTTP 통합 테스트 실행
- `make test`: 두 테스트를 순서대로 실행

## 3. 추가로 넓힐 수 있는 HTTP 테스트

현재 기본 경계는 이미 자동화되어 있으므로, 다음 확장분은 필요할 때 추가하면 된다.

### 3-1. 요청 파싱 경계

- `Content-Length`가 없는 raw request
- 요청 라인이 깨진 raw request
- 헤더가 잘못된 raw request
- body가 중간에 끊긴 요청

이런 케이스는 `curl`만으로는 제한이 있으므로, 별도 raw socket 테스트 스크립트를 추가하는 편이 좋다.

### 3-2. 동시성 경계

- 여러 요청을 동시에 보내도 서버가 살아 있는지 확인
- queue full 상황에서 `503`이 나는지 확인
- insert와 select가 섞여도 row가 누락되지 않는지 확인

### 3-3. 검증 기준

확장 테스트를 추가할 때도 최소한 아래는 유지한다.

- HTTP status code
- `Content-Type`
- JSON 파싱 가능 여부
- 핵심 필드 존재 여부
- 응답 본문 값

### 3-4. 추천 순서

1. 현재의 `smoke_test.sh`와 `http_integration_test.sh`를 먼저 유지한다.
2. 필요해지면 raw request 케이스를 별도 스크립트로 분리한다.
3. 그다음 동시성 테스트를 분리한다.
4. 마지막에 `make` 타겟으로 묶는다.

## 4. 우리가 수동으로 확인하는 방법

이 프로젝트는 Docker 기반이므로, 수동 검증은 `컨테이너를 띄우고, 호스트에서 curl 로 확인하는 방식`이 가장 편합니다.

### 3-1. Docker 이미지 빌드

```bash
docker build -t week8-team2-network .
```

또는 compose 를 쓰면:

```bash
docker compose up --build
```

### 3-2. 서버 실행 확인

컨테이너가 올라오면 로그에 아래와 비슷한 문구가 보여야 합니다.

```text
Listening on port 8080
```

이 메시지가 보이면, 서버가 포트를 열고 대기 상태라는 뜻입니다.

### 3-3. 수동 검증 순서

#### 1) Insert 확인

```bash
curl -v -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "INSERT INTO users VALUES ('Alice', 20);"
```

확인 포인트:

- HTTP `200`
- JSON 응답에 `ok:true`
- `action:"insert"`
- `inserted_id` 존재

#### 2) Select 확인

```bash
curl -v -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "SELECT * FROM users;"
```

확인 포인트:

- HTTP `200`
- `action:"select"`
- `row_count` 가 1 이상
- 방금 넣은 row 가 `rows` 안에 보이는지 확인

#### 3) 비정상 메서드 확인

```bash
curl -v -X GET http://localhost:8080/query
```

확인 포인트:

- HTTP `405`
- 에러 JSON

#### 4) 잘못된 경로 확인

```bash
curl -v -X POST http://localhost:8080/unknown \
  -H "Content-Type: text/plain" \
  --data "SELECT * FROM users;"
```

확인 포인트:

- HTTP `404`

#### 5) 스모크 테스트 실행

```bash
sh scripts/smoke_test.sh
```

이 스크립트는 최소한 다음 흐름을 확인합니다.

- `INSERT`
- `SELECT`

### 3-4. 더 엄격하게 수동 확인하려면

`curl -v` 만 보면 부족할 때가 있습니다. 그럴 때는 아래를 같이 보면 좋습니다.

- 응답 헤더의 `Content-Type`
- 응답 본문의 JSON 형식
- `row_count` 값이 실제 DB 상태와 맞는지
- 여러 번 같은 `INSERT` 를 했을 때 ID가 증가하는지

예를 들어 다음처럼 연속 실행하면 흐름이 더 잘 보입니다.

```bash
curl -s -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "INSERT INTO users VALUES ('Bob', 21);"

curl -s -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "SELECT * FROM users;"
```

## 5. 도커 기반 수동 검증 체크리스트

- 이미지가 `docker build` 로 성공하는가
- 컨테이너가 `8080` 포트에서 열리는가
- `POST /query` 가 정상 동작하는가
- `scripts/http_integration_test.sh`가 실행되는가
- `GET /query` 가 `405` 를 주는가
- `POST /unknown` 이 `404` 를 주는가
- `INSERT` 후 `SELECT` 에서 실제 row 가 보이는가
- 에러 응답도 JSON 형식인가
- `POST /query`에 큰 body를 보내면 `413`이 나오는가
- 서버가 종료 신호를 받았을 때 멈추는가

## 6. 이 문서를 기준으로 다음에 하면 좋은 일

1. `make test` 로 단위 테스트와 HTTP 통합 테스트를 한 번에 실행한다.
2. `README.md` 에도 새 테스트 진입점을 짧게 연결한다.
3. 필요하면 동시성 테스트를 별도 단계로 분리한다.

## 7. 한 줄 요약

이 프로젝트의 HTTP 통합 테스트는 `Docker로 서버를 띄운 뒤, POST /query 성공/실패/경계값을 모두 검증하는 스크립트`로 돌리면 가장 안정적입니다.  
수동 검증은 `docker compose up --build` + `curl -v` + `scripts/smoke_test.sh` 조합으로 재현하고, 전체 자동화는 `make test`로 묶어둘 수 있습니다.
