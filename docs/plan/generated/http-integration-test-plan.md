# HTTP Integration Test Plan

이 문서는 이 프로젝트의 HTTP 통합 테스트를 어떻게 더 추가할지 정리한 계획 문서입니다.

현재 실제로 구현되어 있는 테스트는 `sql_processor` 단위 테스트와 `scripts/smoke_test.sh` 뿐입니다.  
따라서 이 문서는 `현재 있는 테스트`와 `앞으로 추가할 테스트`를 분리해서 적습니다.

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

## 3. 앞으로 추가할 HTTP 통합 테스트

### 3-1. 테스트 레이어를 두 층으로 나누기

추천은 아래 2단계입니다.

#### A. 빠른 스모크 테스트

목적:

- 서버가 띄워지는지 확인
- `INSERT` 후 `SELECT` 가 이어서 동작하는지 확인
- Docker 컨테이너 실행 흐름이 깨지지 않았는지 확인

현재 이미 `scripts/smoke_test.sh` 가 이 역할을 일부 하고 있습니다.

추가하면 좋은 것:

- `SELECT` 결과 JSON에 `ok`, `action`, `row_count`, `rows` 가 있는지 확인
- `INSERT` 응답에 `inserted_id` 가 있는지 확인
- 실패 응답도 하나 포함해서 `404` 또는 `405` 를 같이 확인

#### B. HTTP 통합 테스트 스위트

목적:

- HTTP 파서와 라우팅, SQL 실행, JSON 직렬화까지 한 번에 검증
- 메서드/경로/헤더/본문 크기/SQL 결과를 모두 커버

추천 구조:

- `scripts/http_integration_test.sh`
- 필요하면 `tests/` 또는 `docs/test/demo/fixtures/` 아래에 요청/응답 예시 파일 추가

테스트는 `curl` 또는 `nc` 를 써도 되고, 더 엄격하게 하려면 `python3` 로 응답 JSON을 파싱해 검증하는 방식이 좋습니다.

### 3-2. 꼭 넣으면 좋은 케이스

아래는 자동화 우선순위가 높은 항목입니다.

#### 정상 경로

- `POST /query` + `INSERT INTO users VALUES ('Alice', 20);`
- `POST /query` + `SELECT * FROM users;`
- `POST /query` + `SELECT * FROM users WHERE id = 1;`
- `POST /query` + `SELECT * FROM users WHERE name = 'Alice';`

#### HTTP 에러 경로

- `GET /query` -> `405 Method Not Allowed`
- `POST /unknown` -> `404 Not Found`
- `POST /query` + `Content-Length` 없음 -> `400 Bad Request`
- 요청 라인이 깨진 패킷 -> `400 Bad Request`
- `Content-Length` 가 실제 본문보다 큼 -> `400 Bad Request`
- `Content-Length` 가 허용 크기보다 큼 -> `413 Payload Too Large`

#### SQL 계층 경로

- 문법이 틀린 SQL -> 서버가 죽지 않고 JSON 에러 응답 반환
- `SELECT` 결과가 0건일 때도 응답 포맷이 정상인지 확인
- `EXIT` / `QUIT` 이 HTTP 경로에서 차단되는지 확인

#### 동시성 경로

- 여러 요청을 동시에 날려도 서버가 살아 있는지 확인
- queue full 상황에서 `503` 이 의도대로 나오는지 확인
- 동시 insert 후 row 누락 없이 조회되는지 확인

### 3-3. 자동화 검증 기준

각 테스트는 최소한 아래를 확인하면 좋습니다.

- HTTP status code
- `Content-Type`
- JSON 파싱 가능 여부
- 핵심 필드 존재 여부
- 응답 본문 값

예를 들면 이런 식입니다.

- `INSERT` 성공: `200`, `{"ok":true,"action":"insert","inserted_id":...,"row_count":1}`
- `SELECT` 성공: `200`, `{"ok":true,"action":"select","row_count":...,"rows":[...]}`
- `405` 실패: `405`, 에러 JSON, `status` 값 확인
- `413` 실패: `413`, 에러 JSON, `status` 값 확인

### 3-4. 추천 자동화 순서

1. `scripts/smoke_test.sh` 를 가장 먼저 실행
2. HTTP 통합 테스트 스크립트로 정상/실패 케이스를 분리 실행
3. 동시성 테스트를 마지막에 실행
4. Docker 환경에서 재현 가능하도록 명령을 고정

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
- `GET /query` 가 `405` 를 주는가
- `POST /unknown` 이 `404` 를 주는가
- `INSERT` 후 `SELECT` 에서 실제 row 가 보이는가
- 에러 응답도 JSON 형식인가
- 서버가 종료 신호를 받았을 때 멈추는가

## 6. 이 문서를 기준으로 다음에 하면 좋은 일

1. `scripts/http_integration_test.sh` 를 새로 만든다.
2. 자동화 테스트에서 `curl` 결과를 JSON 단위로 검증한다.
3. `README.md` 에도 `스모크 테스트 + 통합 테스트` 흐름을 짧게 연결한다.
4. 동시성 테스트는 마지막에 별도 단계로 둔다.

## 7. 한 줄 요약

이 프로젝트의 HTTP 통합 테스트는 `Docker로 서버를 띄운 뒤, POST /query 성공/실패/경계값을 모두 검증하는 스크립트`로 만들면 가장 안정적입니다.  
수동 검증은 `docker compose up --build` + `curl -v` + `scripts/smoke_test.sh` 조합이면 충분히 재현 가능합니다.
