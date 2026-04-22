# 8주차 팀 2 네트워크

이 프로젝트는 C로 작성한 메모리 기반 DBMS API 서버입니다.

서버는 단 하나의 HTTP 엔드포인트 `POST /query`를 제공하고, 요청 본문으로 받은 SQL을 기존 `sql_processor` 패키지에 전달합니다.

## 구현된 내용

- C 기반 HTTP 서버 진입점
- `POST /query`만 허용
- `Content-Length`를 사용하는 원시 소켓 요청 파싱
- 제한된 작업자 스레드 풀
- `sql_processor`를 통한 SQL 실행
- JSON 응답 직렬화
- 종료 플래그 기반 graceful shutdown
- Docker 빌드 및 실행 지원

## 빌드

### 로컬

```bash
make db_server
```

### Docker

```bash
docker build -t week8-team2-network .
```

## 실행

### 로컬

```bash
./db_server 8080
```

### Docker

```bash
docker run --rm -p 8080:8080 week8-team2-network
```

또는:

```bash
docker compose up --build
```

## API 사용 예시

### INSERT

```bash
curl -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "INSERT INTO users VALUES ('Alice', 20);"
```

### SELECT

```bash
curl -v -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data-raw "SELECT * FROM users;"
```

## 지원하는 SQL

- `INSERT INTO users VALUES ('Alice', 20);`
- `SELECT * FROM users;`
- `SELECT * FROM users WHERE id = 1;`
- `SELECT * FROM users WHERE id >= 10;`
- `SELECT * FROM users WHERE name = 'Alice';`
- `SELECT * FROM users WHERE age > 20;`

## 테스트

### 스모크 테스트

서버를 띄운 뒤 아래 명령을 실행한다.

```bash
sh scripts/smoke_test.sh
```

### 전체 자동화 테스트

```bash
make test
```

### HTTP 통합 테스트만 실행

```bash
sh scripts/http_integration_test.sh
```

## 프로젝트 문서

- [`docs/plan/00-plan-manager.md`](docs/plan/00-plan-manager.md)
- [`docs/plan/06-implementation-order.md`](docs/plan/06-implementation-order.md)
- [`docs/plan/07-test-and-quality-plan.md`](docs/plan/07-test-and-quality-plan.md)
- [`docs/plan/08-demo-and-readme-plan.md`](docs/plan/08-demo-and-readme-plan.md)
