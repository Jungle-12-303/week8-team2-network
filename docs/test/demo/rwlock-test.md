# rwlock 수동 테스트 안내

이 문서는 `pthread_rwlock_t` 전환 후 직접 확인할 수 있는 절차를 적어둔 안내서다.

## 1. 서버 실행

```bash
./db_server 8080
```

## 2. 기본 INSERT 확인

```bash
curl -v -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data-raw "INSERT INTO users VALUES ('Alice', 20);"
```

기대 결과:
- `ok:true`
- `action:"insert"`
- `inserted_id` 포함

## 3. 기본 SELECT 확인

```bash
curl -v -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data-raw "SELECT * FROM users;"
```

기대 결과:
- `ok:true`
- `action:"select"`
- 방금 넣은 레코드가 `rows`에 포함

## 4. 읽기 병렬성 확인

터미널 여러 개에서 아래 명령을 동시에 실행한다.

```bash
curl -sS -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data-raw "SELECT * FROM users;"
```

여러 `SELECT`가 동시에 실패하지 않고 모두 성공하면 공유 락 경로가 정상이다.

## 5. 읽기/쓰기 혼합 확인

다른 터미널에서 쓰기 요청을 추가한다.

```bash
curl -v -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data-raw "INSERT INTO users VALUES ('Bob', 21);"
```

기대 결과:
- `INSERT`가 실패하지 않아야 한다.
- 이후 `SELECT * FROM users;`에서 `Bob`이 보여야 한다.

## 6. 자동 스모크 테스트

읽기/쓰기 혼합 스트레스는 아래 스크립트로도 확인할 수 있다.

```bash
sh scripts/rwlock_stress_test.sh
```

## 실패 시 확인할 것

- [`server/api.c`](../../server/api.c)에서 `pthread_rwlock_rdlock()` / `pthread_rwlock_wrlock()`이 실제로 호출되는지 확인한다.
- [`server/server.c`](../../server/server.c)에서 `pthread_mutex_t`가 남아 있지 않은지 확인한다.
- `SELECT`가 JSON 변환까지 락을 잡고 있지 않은지 확인한다.

