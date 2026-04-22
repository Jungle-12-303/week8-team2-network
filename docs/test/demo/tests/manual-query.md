# 수동 쿼리 확인

이 문서는 서버를 직접 띄운 뒤 `INSERT`, `SELECT`를 하나씩 입력해서 응답을 눈으로 확인하는 방법을 설명합니다.

## 무엇을 테스트하나

`scripts/manual_query.sh`는 다음 흐름을 확인하기 위한 수동 도구입니다.

- 서버가 이미 떠 있는지 확인한다
- 사용자가 입력한 SQL을 그대로 `/query`로 보낸다
- 응답 JSON을 사람이 보기 쉬운 형태로 출력한다
- `INSERT`, `SELECT`를 한 줄씩 직접 시험해 볼 수 있다

## 사용할 수 있는 쿼리

현재 이 프로젝트에서 직접 넣어 볼 수 있는 SQL은 아래입니다.

- `INSERT INTO users VALUES ('Alice', 20);`
- `INSERT INTO users VALUES ('Bob', 30);`
- `INSERT INTO users VALUES ('Carol', 25);`
- `SELECT * FROM users;`
- `SELECT * FROM users WHERE id = 1;`
- `SELECT * FROM users WHERE id >= 10;`
- `SELECT * FROM users WHERE name = 'Bob';`
- `SELECT * FROM users WHERE age = 20;`
- `SELECT * FROM users WHERE age > 20;`
- `SELECT * FROM users WHERE age <= 20;`

이 외의 문장은 오류 응답이 나올 수 있습니다.

## 수동 실행 순서

### 1) 서버 실행

로컬에서 실행:

```bash
./db_server 8080
```

또는 Docker로 실행:

```bash
docker compose up --build
```

메모리 기반 서버라서, 서버를 껐다가 다시 켜면 기존 row가 모두 초기화됩니다.
예전 테스트 데이터가 너무 많이 쌓였으면 먼저 서버를 재시작한 뒤 시작하면 됩니다.

### 2) 수동 쿼리 도구 실행

대화형으로 실행:

```bash
sh scripts/manual_query.sh 8080
```

그다음 아래처럼 직접 입력한다.

```bash
INSERT INTO users VALUES ('Alice', 20);
INSERT INTO users VALUES ('Bob', 30);
INSERT INTO users VALUES ('Carol', 25);
SELECT * FROM users WHERE id = 1;
SELECT * FROM users WHERE name = 'Bob';
SELECT * FROM users WHERE age = 25;
SELECT * FROM users WHERE age > 20;
SELECT * FROM users WHERE age <= 20;
```

종료하려면 `exit` 또는 `quit`을 입력한다.

### 3) 한 번에 한 쿼리만 실행

```bash
sh scripts/manual_query.sh 8080 "INSERT INTO users VALUES ('Bob', 30);"
sh scripts/manual_query.sh 8080 "SELECT * FROM users WHERE id = 1;"
```

## 기대 결과

- 입력한 SQL이 `SQL>` 뒤에 다시 보인다
- JSON 응답이 들여쓰기된 형태로 출력된다
- `INSERT`는 `ok`, `action`, `inserted_id`, `row_count`를 포함한다
- `SELECT`는 `ok`, `action`, `row_count`, `rows`를 포함한다

## 수동 확인 포인트

- 서버가 실제로 동작 중인가
- `INSERT` 직후 `SELECT`에서 넣은 row가 보이는가
- 응답 JSON이 사람이 읽기 쉬운 형태로 출력되는가
- 잘못된 SQL을 넣었을 때도 응답이 돌아오는가

## 같이 보면 좋은 파일

- [scripts/manual_query.sh](../../../scripts/manual_query.sh)
- [scripts/smoke_test.sh](../../../scripts/smoke_test.sh)
- [scripts/http_integration_test.sh](../../../scripts/http_integration_test.sh)
