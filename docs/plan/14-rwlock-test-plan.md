# 14. rwlock 검증 및 수동 테스트 계획

## 목적

`pthread_rwlock_t` 전환 이후 다음 두 가지를 확인한다.

1. 기능이 기존과 동일하게 동작하는가
2. 읽기 요청이 병렬로 처리되는 구조가 실제로 유지되는가

## 자동 테스트

### 1) 기본 회귀 테스트

- `INSERT`가 정상적으로 저장되는지 확인한다.
- `SELECT * FROM users;`가 기존 결과를 그대로 반환하는지 확인한다.
- `SELECT ... WHERE id = ...`가 단건 조회를 유지하는지 확인한다.
- 잘못된 SQL은 기존처럼 `400` 또는 SQL 에러 JSON으로 응답하는지 확인한다.

### 2) 동시성 스트레스 테스트

- 여러 개의 `SELECT` 요청을 동시에 보낸다.
- 동시에 `INSERT` 요청도 섞어서 보낸다.
- 모든 응답이 JSON 형식으로 정상 반환되는지 확인한다.
- 최종 `SELECT`로 누적된 row 수가 기대값과 일치하는지 확인한다.

## 수동 테스트

### 터미널 1

서버를 실행한다.

```bash
./db_server 8080
```

### 터미널 2

초기 데이터 1건을 넣는다.

```bash
curl -v -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data-raw "INSERT INTO users VALUES ('Alice', 20);"
```

### 터미널 3

짧은 시간에 여러 읽기 요청을 연속으로 보낸다.

```bash
for i in 1 2 3 4 5; do
  curl -sS -X POST http://localhost:8080/query \
    -H "Content-Type: text/plain" \
    --data-raw "SELECT * FROM users;"
done
```

### 터미널 4

읽기 요청이 도는 동안 쓰기 요청도 추가로 보낸다.

```bash
curl -v -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data-raw "INSERT INTO users VALUES ('Bob', 21);"
```

### 기대 결과

- `SELECT` 응답이 여러 개 동시에 성공해야 한다.
- `INSERT` 응답이 실패하지 않아야 한다.
- 마지막 `SELECT * FROM users;`에서 두 레코드가 모두 보여야 한다.

## 자동 검증 스크립트

아래 스크립트는 서버를 띄운 뒤, 읽기/쓰기 요청을 섞어 보내고 최종 row 수를 확인한다.

```bash
sh scripts/rwlock_stress_test.sh
```

## 실패 시 확인할 것

- `api_handle_query()`가 아직 `pthread_mutex_t`를 쓰고 있지 않은지 확인한다.
- `SELECT`와 `INSERT`가 각각 read lock / write lock을 타는지 확인한다.
- JSON 응답이 락 안에서 만들어지지 않도록 했는지 확인한다.

