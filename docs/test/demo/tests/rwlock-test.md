# rwlock 수동 테스트 안내

이 문서는 `pthread_rwlock_t` 전환 이후 무엇을 확인하는지,  
그리고 `scripts/tests/concurrency/rwlock-stress-test.sh`가 실제로 어떤 상황을 검증하는지 자세히 설명한다.

---

## 1. 이 테스트가 보는 것

이 테스트의 핵심은 아주 단순하다.

- `SELECT`가 `read lock` 경로를 타는지
- `INSERT`가 `write lock` 경로를 타는지
- 여러 요청을 동시에 보내도 서버가 깨지지 않는지
- 최종적으로 데이터가 빠지지 않고 모두 반영되는지

즉, 이 테스트는 "락이 실제로 분리되어 동작하는가"와 "동시 요청을 버틸 수 있는가"를 확인한다.

---

## 2. `rwlock`을 왜 테스트하나

`pthread_rwlock_t`의 핵심은 다음과 같다.

- 읽기 요청은 여러 개가 동시에 들어올 수 있다
- 쓰기 요청은 한 번에 하나만 들어온다
- 쓰기 중에는 읽기도 막힌다
- 읽기 중에는 쓰기도 기다린다

그래서 `rwlock-stress-test`는 단순히 "응답이 온다"만 보는 게 아니라,
읽기와 쓰기가 섞인 상황에서도 데이터가 정상적으로 누적되는지를 본다.

---

## 3. 테스트 환경을 정확히 구분하기

이 테스트는 "터미널에 SQL을 직접 입력하는 방식"이 아니다.

정확히는 아래 두 가지 환경을 구분해야 한다.

- **HTTP 클라이언트 기반 테스트**: `curl`로 서버에 HTTP 요청을 보내는 방식
- **SQL REPL 기반 테스트**: `sql_processor/main`을 실행해서 SQL을 직접 입력하는 방식

여기서 `rwlock-stress-test.sh`는 **HTTP 클라이언트 기반 테스트**다.
즉, SQL을 쉘에 직접 치는 것이 아니라, `curl`이 SQL 문자열을 HTTP 요청 본문으로 서버에 보내는 구조다.

---

## 4. HTTP 클라이언트(curl) 기반 수동 테스트

이 방법은 현재 서버 구현을 직접 검증할 때 가장 중요하다.

### 4-1. 서버 빌드

```bash
make db_server
```

### 4-2. 서버 실행

```bash
./db_server 18080
```

### 4-3. 초기 데이터 1건 삽입

```bash
curl -sS -X POST http://localhost:18080/query \
  -H "Content-Type: text/plain" \
  --data-raw "INSERT INTO users VALUES ('Seed', 1);"
```

### 4-4. 읽기 요청 확인

```bash
curl -sS -X POST http://localhost:18080/query \
  -H "Content-Type: text/plain" \
  --data-raw "SELECT * FROM users;"
```

### 4-5. 읽기와 쓰기를 동시에 보내기

한쪽은 조회, 한쪽은 삽입 요청을 동시에 날린다.

```bash
curl -sS -X POST http://localhost:18080/query \
  -H "Content-Type: text/plain" \
  --data-raw "SELECT * FROM users;" &

curl -sS -X POST http://localhost:18080/query \
  -H "Content-Type: text/plain" \
  --data-raw "INSERT INTO users VALUES ('Bob', 21);" &
wait
```

이 방식은 "읽기와 쓰기가 같은 시간대에 들어와도 서버가 정상 동작하는지"를 보는 최소 예시다.

---

## 5. SQL REPL 기반 수동 테스트

이 방법은 서버가 아니라 `sql_processor` 자체를 직접 확인할 때 쓰는 방식이다.

### 5-1. REPL 실행

```bash
cd sql_processor
./main
```

### 5-2. REPL에서 직접 SQL 입력

```sql
INSERT INTO users VALUES ('Seed', 1);
SELECT * FROM users;
```

이 경우는 SQL 문을 셸에 입력하는 것이 아니라,  
REPL 프로그램의 입력창에 SQL을 직접 넣는 방식이다.

### 5-3. 이 방식의 의미

- SQL 파서가 잘 동작하는지 확인할 수 있다
- B+Tree 기반 `id` 조회가 잘 되는지 볼 수 있다
- HTTP 서버를 거치지 않고 SQL 엔진만 따로 확인할 수 있다

---

## 6. `rwlock-stress-test.sh`가 실제로 하는 일

스크립트는 아래 순서로 움직인다.

### 6-1. 서버 빌드

```bash
make db_server
```

### 6-2. 서버 실행

```bash
./db_server 18080
```

### 6-3. 서버 준비 확인

서버가 바로 떠 있지 않을 수 있으므로, 스크립트는 여러 번 재시도하면서 서버 준비 여부를 확인한다.

### 6-4. 초기 데이터 1건 삽입

```bash
INSERT INTO users VALUES ('Seed', 1);
```

### 6-5. 읽기와 쓰기를 동시에 10회씩 전송

각 반복마다 아래 두 요청을 동시에 보낸다.

- `SELECT * FROM users;`
- `INSERT INTO users VALUES ('UserN', 20 + N);`

총 10번 반복하므로,

- 읽기 요청 10개
- 쓰기 요청 10개

가 거의 동시에 날아간다.

### 6-6. 모든 응답이 성공인지 확인

각 요청의 응답에서 다음 조건을 검사한다.

- JSON 응답에 `ok:true`가 들어 있는지
- 요청이 중간에 실패하지 않았는지

### 6-7. 마지막으로 전체 row 수 확인

마지막에 다시 한 번:

```bash
SELECT * FROM users;
```

를 보내고, `row_count`가 정확히 `11`인지 확인한다.

이 값이 11이어야 하는 이유는:

- 시작 시 seed 1건
- 반복 INSERT 10건
- 합계 11건

이기 때문이다.

---

## 7. 이 테스트가 검증하는 것

### 7-1. 읽기 락이 동작하는지

여러 `SELECT` 요청이 동시에 들어올 때 서버가 정상적으로 처리되는지를 본다.

### 7-2. 쓰기 락이 동작하는지

여러 `INSERT` 요청이 동시에 들어와도 데이터가 꼬이지 않고 순서대로 반영되는지를 본다.

### 7-3. 읽기와 쓰기가 섞여도 데이터가 유지되는지

읽기와 쓰기를 섞어 보낸 뒤에도 최종 결과가 기대한 row 수를 가지는지 확인한다.

### 7-4. 서버가 동시에 많이 들어오는 요청을 버티는지

락 경쟁이 생기는 상황에서도 서버가 죽지 않고 응답을 끝까지 돌려주는지를 본다.

---

## 8. `read lock`과 `write lock`을 이렇게 이해하면 된다

- `read lock` = 공유락
  - 여러 읽기 요청이 같이 들어갈 수 있다
  - 서로 데이터를 바꾸지 않기 때문에 동시에 가능하다

- `write lock` = 배타락
  - 쓰기 요청은 한 번에 하나만 들어간다
  - 데이터를 바꾸는 동안에는 다른 읽기/쓰기 요청이 기다린다

즉, `rwlock`은:

- 읽는 사람끼리는 같이 허용하고
- 쓰는 사람은 혼자만 허용하는

락이다.

---

## 9. 테스트 로그를 읽는 방법

스크립트가 마지막에 성공하면 다음 문구를 출력한다.

```text
rwlock stress test passed.
```

이 메시지는 아래를 의미한다.

- 모든 요청이 `ok:true`로 끝났다
- 최종 `SELECT`의 `row_count`가 11이었다
- 요청 폭주 상황에서도 서버가 정상적으로 버텼다

---

## 10. 이 테스트가 보장하지 않는 것

이 테스트는 락이 잘 동작하는지 확인하는 데는 좋지만,  
아래까지 완전히 증명하진 않는다.

- 정확한 초 단위 성능 수치
- 모든 가능한 race condition
- 메모리 안전성 전체
- 아주 극단적인 네트워크 실패 상황

즉, 이 테스트는 "기본 동시성 동작이 정상인지"를 보는 실전형 확인용이다.

---

## 11. 코드에서 직접 확인할 포인트

다음 파일을 함께 보면 이해가 더 쉽다.

- [`scripts/tests/concurrency/rwlock-stress-test.sh`](../../../scripts/tests/concurrency/rwlock-stress-test.sh)
- [`server/api.c`](../../../server/api.c)
- [`server/server.c`](../../../server/server.c)
- [`sql_processor/sql.c`](../../../sql_processor/sql.c)

특히 확인할 점은 아래다.

- `api_handle_query()`가 SQL 종류에 따라 `read lock` 또는 `write lock`을 고르는지
- `server.c`가 `pthread_rwlock_t`를 실제로 초기화하고 있는지
- `SELECT`와 `INSERT`가 모두 성공 응답을 반환하는지

---

## 12. 한 줄 요약

`rwlock-stress-test`는 `SELECT`와 `INSERT`를 동시에 많이 던져도,
읽기/쓰기 락이 올바르게 분리되어 동작하고 최종 데이터 개수가 정확히 맞는지 확인하는 테스트다.
