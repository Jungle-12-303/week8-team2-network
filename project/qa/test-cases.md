# MVP API 테스트 케이스

## 목적

이 문서는 미니 DBMS API 서버 MVP의 기능 검증 범위를 정의한다.

검증 대상은 아래 4개 endpoint다.

- `GET /health`
- `POST /users`
- `GET /users`
- `GET /users/{id}`

이번 테스트 문서는 최소 구현 플랜 기준으로 작성한다.
즉, 제한된 HTTP subset, `Connection: close`, JSON request/response, 단일
`users` 테이블, thread pool 기반 처리, coarse-grained DB lock 전략을 전제로
한다.

## 공통 전제

- 서버는 로컬에서 실행 중이다.
- 기본 주소는 `http://127.0.0.1:8081`로 가정한다.
- DB는 메모리 기반이며 서버 재시작 시 초기화될 수 있다.
- 요청은 기본적으로 요청당 연결 1회 처리 방식이다.
- 응답 body는 JSON 형식을 사용한다.
- 상태 코드는 서버 구현에 따라 세부 메시지가 달라질 수 있으나, 본 문서의
  기대 결과를 기준으로 검증한다.

## 공통 확인 항목

각 테스트 케이스에서 아래를 함께 확인한다.

- 서버 프로세스가 종료되지 않는지
- 응답 상태 코드가 기대값과 일치하는지
- 응답 body가 JSON 형식인지
- 실패 케이스에서 에러 응답 형식이 일관적인지
- 동일 요청 반복 시 재현 가능하게 동작하는지

## 테스트 환경 권장값

- OS: macOS 또는 Linux
- 빌드: `make`
- 실행: `./build/server` 또는 프로젝트 README에 정의된 실행 명령
- 기본 검증 도구:
  - `curl`
  - 쉘 반복문
  - 필요 시 `xargs -P`, `ab`, `wrk` 중 하나

## 1. GET /health

### 정상 케이스

#### TC-HEALTH-001 서버 생존 확인

- 목적: 서버가 정상 실행 중일 때 health check가 성공한다.
- 요청:

```bash
curl -i -sS http://127.0.0.1:8081/health
```

- 기대 결과:
  - `200 OK`
  - JSON 응답 반환
  - 최소한 `{"ok": true}` 또는 동일 의미 필드 포함

#### TC-HEALTH-002 연속 호출 안정성

- 목적: 동일 endpoint를 여러 번 호출해도 안정적으로 응답한다.
- 요청:

```bash
for i in $(seq 1 20); do
  curl -sS http://127.0.0.1:8081/health > /dev/null || exit 1
done
```

- 기대 결과:
  - 모든 호출 성공
  - 서버 비정상 종료 없음

### 에러 케이스

#### TC-HEALTH-003 잘못된 method

- 목적: 지원하지 않는 method에 대해 적절히 실패한다.
- 요청:

```bash
curl -i -sS -X POST http://127.0.0.1:8081/health
```

- 기대 결과:
  - `400 Bad Request` 또는 `405 Method Not Allowed`
  - JSON 에러 응답 반환

### 경계 케이스

#### TC-HEALTH-004 불필요한 body 포함

- 목적: body가 없어야 하는 요청에 body가 와도 서버가 죽지 않는다.
- 요청:

```bash
curl -i -sS -X GET \
  -H 'Content-Type: application/json' \
  -d '{"ignored":true}' \
  http://127.0.0.1:8081/health
```

- 기대 결과:
  - 서버 비정상 종료 없음
  - 구현 정책에 따라 성공 또는 실패 가능
  - 어떤 경우든 일관된 상태 코드와 JSON 응답 반환

### 동시성 케이스

#### TC-HEALTH-005 동시 health check

- 목적: 여러 worker가 동시에 health 요청을 처리해도 정상 동작한다.
- 요청:

```bash
seq 1 16 | xargs -I{} -P 8 curl -sS http://127.0.0.1:8081/health > /dev/null
```

- 기대 결과:
  - 모든 요청 성공
  - 서버 hang, crash, 비정상 응답 없음

## 2. POST /users

### 정상 케이스

#### TC-POST-001 사용자 생성 성공

- 목적: 유효한 JSON body로 사용자 생성이 성공한다.
- 요청:

```bash
curl -i -sS -X POST http://127.0.0.1:8081/users \
  -H 'Content-Type: application/json' \
  -d '{"name":"Alice","age":20}'
```

- 기대 결과:
  - `201 Created`
  - JSON 응답 반환
  - 최소한 `ok: true` 또는 `message: "created"` 포함

#### TC-POST-002 연속 생성 성공

- 목적: 여러 명의 사용자를 순차적으로 생성할 수 있다.
- 요청:

```bash
curl -sS -X POST http://127.0.0.1:8081/users \
  -H 'Content-Type: application/json' \
  -d '{"name":"Alice","age":20}'

curl -sS -X POST http://127.0.0.1:8081/users \
  -H 'Content-Type: application/json' \
  -d '{"name":"Bob","age":21}'
```

- 기대 결과:
  - 두 요청 모두 성공
  - 이후 `GET /users`에서 생성 결과 확인 가능

### 에러 케이스

#### TC-POST-003 body 누락

- 목적: body 없이 생성 요청 시 검증 오류를 반환한다.
- 요청:

```bash
curl -i -sS -X POST http://127.0.0.1:8081/users
```

- 기대 결과:
  - `400 Bad Request`
  - JSON 에러 응답 반환

#### TC-POST-004 malformed JSON

- 목적: 잘못된 JSON 형식을 감지한다.
- 요청:

```bash
curl -i -sS -X POST http://127.0.0.1:8081/users \
  -H 'Content-Type: application/json' \
  -d '{"name":"Alice","age":20'
```

- 기대 결과:
  - `400 Bad Request`
  - 서버 crash 없음

#### TC-POST-005 필수 필드 누락

- 목적: `name` 또는 `age` 누락 시 실패한다.
- 요청:

```bash
curl -i -sS -X POST http://127.0.0.1:8081/users \
  -H 'Content-Type: application/json' \
  -d '{"name":"Alice"}'
```

- 기대 결과:
  - `400 Bad Request`
  - 어떤 필드가 잘못됐는지 추적 가능한 에러 메시지 또는 공통 에러 응답

#### TC-POST-006 지원하지 않는 method 또는 path

- 목적: 잘못된 요청이 생성 로직으로 흘러가지 않는다.
- 요청:

```bash
curl -i -sS -X PUT http://127.0.0.1:8081/users \
  -H 'Content-Type: application/json' \
  -d '{"name":"Alice","age":20}'
```

- 기대 결과:
  - `400 Bad Request`, `404 Not Found`, 또는 `405 Method Not Allowed`
  - 서버 비정상 종료 없음

### 경계 케이스

#### TC-POST-007 빈 문자열 이름

- 목적: `name`이 빈 문자열일 때 정책에 맞게 처리한다.
- 요청:

```bash
curl -i -sS -X POST http://127.0.0.1:8081/users \
  -H 'Content-Type: application/json' \
  -d '{"name":"","age":20}'
```

- 기대 결과:
  - 정책에 따라 `400 Bad Request` 또는 생성 성공
  - 어떤 경우든 동작이 일관적이어야 한다

#### TC-POST-008 음수 나이

- 목적: 비정상 입력을 거부하거나 일관되게 처리한다.
- 요청:

```bash
curl -i -sS -X POST http://127.0.0.1:8081/users \
  -H 'Content-Type: application/json' \
  -d '{"name":"Alice","age":-1}'
```

- 기대 결과:
  - 정책에 따라 `400 Bad Request` 또는 생성 성공
  - 허용 여부가 문서 또는 구현과 일치해야 한다

#### TC-POST-009 큰 본문 또는 긴 이름

- 목적: 입력 길이 경계에서 서버가 비정상 종료되지 않는다.
- 요청:
  - 긴 이름 문자열 또는 비교적 큰 body로 생성 요청
- 기대 결과:
  - 서버 crash 없음
  - 버퍼 초과, 메모리 오류, 잘린 응답이 없어야 한다
  - 정책에 따라 성공 또는 검증 실패 가능

### 동시성 케이스

#### TC-POST-010 동시 생성 요청

- 목적: 여러 worker가 동시에 생성 요청을 받아도 DB 무결성이 깨지지 않는다.
- 요청:

```bash
seq 1 8 | xargs -I{} -P 8 sh -c \
'curl -sS -X POST http://127.0.0.1:8081/users \
  -H "Content-Type: application/json" \
  -d "{\"name\":\"user{}\",\"age\":20}" > /dev/null'
```

- 기대 결과:
  - 서버 crash 없음
  - 성공한 요청 수만큼 데이터가 조회 가능
  - 데이터 손실, 중복 이상 징후가 없는지 확인

## 3. GET /users

### 정상 케이스

#### TC-GETALL-001 전체 조회 성공

- 목적: 생성된 사용자 목록을 조회할 수 있다.
- 선행 조건:
  - `POST /users`로 1건 이상 생성
- 요청:

```bash
curl -i -sS http://127.0.0.1:8081/users
```

- 기대 결과:
  - `200 OK`
  - JSON 응답 반환
  - `rows` 또는 동등한 배열 필드 포함
  - 생성한 사용자 데이터가 응답에 포함

#### TC-GETALL-002 데이터가 없는 상태 조회

- 목적: 초기 상태 또는 비어 있는 상태에서도 안정적으로 응답한다.
- 요청:

```bash
curl -i -sS http://127.0.0.1:8081/users
```

- 기대 결과:
  - `200 OK`
  - 빈 배열 또는 `row_count: 0` 형태 응답

### 에러 케이스

#### TC-GETALL-003 잘못된 method

- 목적: 지원하지 않는 method를 거부한다.
- 요청:

```bash
curl -i -sS -X POST http://127.0.0.1:8081/users
```

- 기대 결과:
  - `400 Bad Request` 또는 `405 Method Not Allowed`
  - JSON 에러 응답 반환

### 경계 케이스

#### TC-GETALL-004 반복 조회 안정성

- 목적: 전체 조회를 여러 번 반복해도 결과 형식이 깨지지 않는다.
- 요청:

```bash
for i in $(seq 1 30); do
  curl -sS http://127.0.0.1:8081/users > /dev/null || exit 1
done
```

- 기대 결과:
  - 모든 요청 성공
  - 서버 메모리 오류나 비정상 종료 없음

### 동시성 케이스

#### TC-GETALL-005 동시 전체 조회

- 목적: read lock 기반 병렬 조회가 안정적으로 수행된다.
- 요청:

```bash
seq 1 20 | xargs -I{} -P 8 curl -sS http://127.0.0.1:8081/users > /dev/null
```

- 기대 결과:
  - 모든 요청 성공
  - 응답 누락, 서버 hang, crash 없음

#### TC-GETALL-006 생성과 조회 혼합 부하

- 목적: write lock과 read lock이 함께 동작할 때 무결성이 유지된다.
- 요청:

```bash
seq 1 5 | xargs -I{} -P 5 sh -c \
'curl -sS -X POST http://127.0.0.1:8081/users \
  -H "Content-Type: application/json" \
  -d "{\"name\":\"mix{}\",\"age\":30}" > /dev/null'

seq 1 10 | xargs -I{} -P 5 curl -sS http://127.0.0.1:8081/users > /dev/null
```

- 기대 결과:
  - 서버 crash 없음
  - 조회 결과가 깨지지 않음
  - 완료 후 전체 데이터 개수가 합리적인 범위에 있음

## 4. GET /users/{id}

### 정상 케이스

#### TC-GETONE-001 단건 조회 성공

- 목적: 존재하는 `id`로 단건 조회가 성공한다.
- 선행 조건:
  - 먼저 사용자 1건 이상 생성
- 요청:

```bash
curl -i -sS http://127.0.0.1:8081/users/1
```

- 기대 결과:
  - `200 OK`
  - JSON 응답 반환
  - 요청한 `id`에 해당하는 사용자 정보 포함

### 에러 케이스

#### TC-GETONE-002 존재하지 않는 id

- 목적: 없는 리소스에 대해 적절히 실패한다.
- 요청:

```bash
curl -i -sS http://127.0.0.1:8081/users/999999
```

- 기대 결과:
  - `404 Not Found`
  - JSON 에러 응답 반환

#### TC-GETONE-003 숫자가 아닌 id

- 목적: path parameter 검증이 동작한다.
- 요청:

```bash
curl -i -sS http://127.0.0.1:8081/users/abc
```

- 기대 결과:
  - `400 Bad Request` 또는 `404 Not Found`
  - 서버 비정상 종료 없음

#### TC-GETONE-004 잘못된 path 형식

- 목적: path routing 오류를 적절히 처리한다.
- 요청:

```bash
curl -i -sS http://127.0.0.1:8081/users/1/extra
```

- 기대 결과:
  - `404 Not Found` 또는 `400 Bad Request`
  - JSON 에러 응답 반환

### 경계 케이스

#### TC-GETONE-005 최소값 경계

- 목적: `id` 경계값 입력에서 안정적으로 동작한다.
- 요청:

```bash
curl -i -sS http://127.0.0.1:8081/users/0
```

- 기대 결과:
  - 정책에 따라 `404 Not Found` 또는 `400 Bad Request`
  - 서버 crash 없음

#### TC-GETONE-006 큰 정수 id

- 목적: 매우 큰 숫자 path parameter에 대해 안전하게 처리한다.
- 요청:

```bash
curl -i -sS http://127.0.0.1:8081/users/2147483647
```

- 기대 결과:
  - `404 Not Found` 또는 `400 Bad Request`
  - 정수 파싱 오류로 인한 비정상 종료 없음

### 동시성 케이스

#### TC-GETONE-007 동일 id 동시 조회

- 목적: 동일 리소스에 대한 병렬 read가 안정적이다.
- 요청:

```bash
seq 1 20 | xargs -I{} -P 8 curl -sS http://127.0.0.1:8081/users/1 > /dev/null
```

- 기대 결과:
  - 모든 요청 성공
  - 응답 형식이 일관적임

#### TC-GETONE-008 생성 중 단건 조회

- 목적: 쓰기와 읽기가 섞여도 서버가 죽지 않고 일관성을 유지한다.
- 요청:

```bash
seq 1 5 | xargs -I{} -P 5 sh -c \
'curl -sS -X POST http://127.0.0.1:8081/users \
  -H "Content-Type: application/json" \
  -d "{\"name\":\"race{}\",\"age\":40}" > /dev/null'

seq 1 10 | xargs -I{} -P 5 curl -sS http://127.0.0.1:8081/users/1 > /dev/null
```

- 기대 결과:
  - 서버 crash 없음
  - 조회 응답 JSON 형식 유지

## 회귀 우선순위

변경이 발생했을 때 아래 순서로 우선 재검증한다.

1. `TC-HEALTH-001`
2. `TC-POST-001`
3. `TC-GETALL-001`
4. `TC-GETONE-001`
5. `TC-POST-010`
6. `TC-GETALL-006`

## 최소 합격 기준

아래 조건을 모두 만족하면 MVP 기준 테스트를 통과한 것으로 본다.

- 핵심 endpoint 4개가 정상 케이스를 통과한다.
- 대표 에러 케이스에서 서버가 죽지 않는다.
- 동시 조회와 생성/조회 혼합 요청에서 crash, hang, 명백한 데이터 손상이 없다.
- 재현 절차와 결과를 `project/qa/test-results-template.md` 기준으로 기록할 수 있다.
