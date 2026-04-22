# 6단계. 테스트와 데모 준비하기

## 목표

발표 중 터미널 데모를 안정적으로 수행하고, 테스트 결과를 근거 있게 설명할 수 있도록 준비합니다.

## 예상 소요 시간

- 40~50분

## 데모 원칙

- [ ] 발표 전에는 반드시 모든 명령을 직접 실행한다.
- [ ] 데모 중에는 명령을 복사해서 붙여넣을 수 있게 준비한다.
- [ ] 서버 실행 터미널과 요청 실행 터미널을 분리한다.
- [ ] 동시 요청 결과는 순서가 섞일 수 있음을 알고 있어야 한다.
- [ ] 데모가 실패할 경우를 대비해 테스트 결과 문장을 준비한다.

## 6-1. 서버 빌드 검증

프로젝트 루트에서 실행합니다.

- [ ] clean build를 실행한다.

```bash
make clean && make db_server
```

확인할 것:

- [ ] 컴파일이 성공하는가?
- [ ] `db_server`가 생성되는가?
- [ ] warning이 error로 처리되는 설정에서도 통과하는가?

체크 질문:

- [ ] `make clean`을 먼저 하는 이유는 무엇인가?
- [ ] 서버 실행 파일 이름은 무엇인가?
- [ ] 서버 실행 파일에 SQL 처리기 코드가 함께 링크되는가?

발표 문장:

```text
서버는 루트 Makefile에서 db_server target으로 빌드했고, server 코드와 sql_processor 코드를 함께 링크합니다.
```

## 6-2. SQL 단위 테스트 검증

SQL 처리기 디렉터리에서 실행합니다.

- [ ] `sql_processor`로 이동한다.

```bash
cd sql_processor
```

- [ ] unit test를 빌드하고 실행한다.

```bash
make clean && make unit_test && ./unit_test
```

- [ ] 루트로 돌아온다.

```bash
cd ..
```

기대 출력:

```text
All unit tests passed.
```

검증하는 것:

- [ ] B+Tree empty search
- [ ] B+Tree insert/search
- [ ] duplicate key reject
- [ ] leaf split
- [ ] internal split
- [ ] table auto increment
- [ ] id 조회
- [ ] name/age 선형 조회
- [ ] 조건 조회
- [ ] SQL insert/select
- [ ] SQL error response

체크 질문:

- [ ] 이 unit test는 API 서버까지 테스트하는가?
- [ ] 이 unit test는 어떤 계층을 검증하는가?

답변 힌트:

```text
sql_processor/unit_test는 API 서버가 아니라 SQL 처리기, Table, B+Tree를 검증합니다.
```

## 6-3. 서버 실행 준비

루트에서 실행합니다.

- [ ] 서버를 실행한다.

```bash
./db_server 8080
```

확인할 것:

- [ ] `Listening on port 8080`이 출력되는가?
- [ ] 서버가 계속 실행 중인가?
- [ ] 이 터미널은 그대로 둔다.

포트 충돌 시:

- [ ] 다른 포트를 사용한다.

```bash
./db_server 18080
```

- [ ] curl URL과 smoke test 포트도 같이 바꾼다.

```bash
sh scripts/smoke_test.sh 18080
```

체크 질문:

- [ ] 서버 포트는 어디에서 결정되는가?
- [ ] 포트를 생략하면 기본값은 무엇인가?

## 6-4. smoke test 데모

다른 터미널에서 실행합니다.

- [ ] smoke test를 실행한다.

```bash
sh scripts/smoke_test.sh 8080
```

확인할 것:

- [ ] INSERT 응답이 출력된다.
- [ ] SELECT 응답이 출력된다.
- [ ] SELECT 결과에 Alice가 들어 있다.

발표 문장:

```text
smoke test는 INSERT 한 번과 SELECT 한 번을 보내서 API 서버와 SQL 엔진 연결이 정상인지 빠르게 확인합니다.
```

체크 질문:

- [ ] smoke test는 concurrency를 검증하는가?
- [ ] smoke test는 오류 응답을 검증하는가?
- [ ] smoke test의 장점은 무엇인가?

답변 힌트:

```text
smoke test는 전체 시스템이 기본적으로 살아 있는지 확인하는 빠른 기능 테스트입니다.
```

## 6-5. 직접 INSERT 데모

- [ ] INSERT 요청을 실행한다.

```bash
curl -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "INSERT INTO users VALUES ('Bob', 30);"
```

확인할 것:

- [ ] `ok:true`
- [ ] `action:"insert"`
- [ ] `inserted_id`
- [ ] `row_count:1`

발표 문장:

```text
INSERT 요청은 SQL 문자열을 body로 보내고, 응답으로 새로 생성된 inserted_id와 row_count를 받습니다.
```

## 6-6. 직접 SELECT 데모

- [ ] 전체 SELECT를 실행한다.

```bash
curl -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "SELECT * FROM users;"
```

확인할 것:

- [ ] `action:"select"`
- [ ] `row_count`
- [ ] `rows` 배열
- [ ] row 안의 `id`, `name`, `age`

발표 문장:

```text
SELECT 결과는 row_count와 rows 배열로 반환됩니다.
```

## 6-7. B+Tree 조건 조회 데모

- [ ] id range query를 실행한다.

```bash
curl -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "SELECT * FROM users WHERE id >= 1;"
```

설명할 것:

- [ ] `id`는 B+Tree index가 있다.
- [ ] `id >= 1`은 B+Tree leaf에서 시작 위치를 찾고 leaf chain을 따라간다.
- [ ] 결과는 JSON rows 배열이다.

발표 문장:

```text
id 조건 조회는 B+Tree index를 사용합니다. 특히 range query는 leaf node의 연결을 따라가며 결과를 모읍니다.
```

## 6-8. 선형 탐색 조건 조회 데모

- [ ] name 조건 조회를 실행한다.

```bash
curl -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "SELECT * FROM users WHERE name = 'Alice';"
```

- [ ] age 조건 조회를 실행한다.

```bash
curl -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "SELECT * FROM users WHERE age > 20;"
```

설명할 것:

- [ ] name에는 별도 index가 없다.
- [ ] age에도 별도 index가 없다.
- [ ] 따라서 rows 배열을 선형 탐색한다.

발표 문장:

```text
name과 age는 secondary index가 없기 때문에 rows 배열을 순회하는 선형 탐색으로 처리합니다.
```

## 6-9. 동시 INSERT 데모

- [ ] 20개 INSERT를 병렬로 보낸다.

```bash
seq 1 20 | xargs -n1 -P8 -I{} curl -s -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "INSERT INTO users VALUES ('User{}', {});"
```

확인할 것:

- [ ] 서버가 죽지 않는다.
- [ ] 모든 응답이 `ok:true`다.
- [ ] `inserted_id`가 중복되지 않는다.
- [ ] 응답 순서는 섞일 수 있다.

- [ ] 최종 SELECT로 데이터가 유지되는지 확인한다.

```bash
curl -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "SELECT * FROM users;"
```

발표 문장:

```text
동시 INSERT 요청을 보내도 DB mutex가 shared Table을 보호하기 때문에 inserted_id가 중복되지 않고 서버가 유지됩니다.
```

주의 문장:

```text
동시 요청이므로 응답 출력 순서는 입력 순서와 다를 수 있습니다. 중요한 검증 기준은 중복 id 없이 최종 데이터가 유지되는 것입니다.
```

## 6-10. SQL 오류 응답 데모

- [ ] 잘못된 SQL을 보낸다.

```bash
curl -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "SELECT * FORM users;"
```

확인할 것:

- [ ] 서버가 crash하지 않는다.
- [ ] `ok:false`
- [ ] `status:"syntax_error"`
- [ ] `error_code`
- [ ] `sql_state`
- [ ] `message`

발표 문장:

```text
잘못된 SQL은 서버 crash가 아니라 SQLResult 오류로 변환되고, JSON 오류 응답으로 반환됩니다.
```

## 6-11. HTTP 오류 응답 데모

시간이 남을 때만 발표에서 보여줍니다.

- [ ] GET 요청을 보낸다.

```bash
curl -i -X GET http://localhost:8080/query
```

기대:

- [ ] 405 Method Not Allowed

- [ ] 잘못된 path로 요청한다.

```bash
curl -i -X POST http://localhost:8080/bad \
  -H "Content-Type: text/plain" \
  --data "SELECT * FROM users;"
```

기대:

- [ ] 404 Not Found

체크 질문:

- [ ] SQL 오류와 HTTP 오류는 왜 구분해야 하는가?

답변 힌트:

```text
HTTP 오류는 요청 method/path/body 크기처럼 API 계약 자체가 틀린 경우이고, SQL 오류는 API 요청은 정상적으로 들어왔지만 SQL 엔진에서 실행할 수 없는 경우입니다.
```

## 6-12. 데모 순서 최종본

발표 중 추천 순서:

- [ ] 서버 빌드 결과 언급
- [ ] unit test 결과 언급
- [ ] 서버 실행
- [ ] smoke test
- [ ] INSERT
- [ ] SELECT
- [ ] `WHERE id >= 1`
- [ ] 동시 INSERT
- [ ] 최종 SELECT
- [ ] 시간이 남으면 오류 SQL

데모 명령 모음:

```bash
make clean && make db_server
```

```bash
cd sql_processor && make clean && make unit_test && ./unit_test && cd ..
```

```bash
./db_server 8080
```

```bash
sh scripts/smoke_test.sh 8080
```

```bash
curl -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "SELECT * FROM users WHERE id >= 1;"
```

```bash
seq 1 20 | xargs -n1 -P8 -I{} curl -s -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "INSERT INTO users VALUES ('User{}', {});"
```

## 완료 기준

- [ ] 서버 clean build를 성공했다.
- [ ] SQL unit test를 성공했다.
- [ ] 서버를 실행했다.
- [ ] smoke test를 성공했다.
- [ ] INSERT, SELECT, 조건 조회를 직접 실행했다.
- [ ] 동시 INSERT를 직접 실행했다.
- [ ] 최종 SELECT로 데이터 유지를 확인했다.
- [ ] SQL 오류 JSON 응답을 확인했다.
- [ ] 데모 중 설명할 문장을 준비했다.
