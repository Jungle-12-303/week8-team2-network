# MVP API Spec

## 범위

이번 주 최소 구현 범위의 API는 아래 네 개로 제한한다.

- `GET /health`
- `POST /users`
- `GET /users`
- `GET /users/{id}`

지원하지 않는 항목:

- query parameter 기반 필터 조회
- 복수 테이블
- 인증/인가
- 외부 SQL 직접 실행 API

## 공통 규칙

- 프로토콜은 제한된 `HTTP/1.1` subset을 사용한다.
- 요청당 연결 1회 처리 후 서버가 연결을 닫는다.
- 모든 응답은 JSON 형식으로 반환한다.
- `POST /users`는 `Content-Length`가 있는 JSON body만 지원한다.

## 1. `GET /health`

### 목적

서버 프로세스가 요청을 수신하고 응답 가능한 상태인지 확인한다.

### 요청

```http
GET /health HTTP/1.1
Host: localhost:8080
```

### 성공 응답

- 상태 코드: `200 OK`

```json
{
  "ok": true
}
```

## 2. `POST /users`

### 목적

새 사용자를 생성한다.

### 요청

```http
POST /users HTTP/1.1
Content-Type: application/json
Content-Length: ...
```

```json
{
  "name": "Alice",
  "age": 20
}
```

### 내부 SQL 변환

```sql
INSERT INTO users VALUES ('Alice', 20);
```

### 성공 응답

- 상태 코드: `201 Created`

```json
{
  "ok": true,
  "message": "created"
}
```

### 실패 응답

- body가 비어 있거나 JSON 파싱 실패: `400 Bad Request`
- 필수 필드 누락: `400 Bad Request`
- 내부 엔진 실행 실패: `500 Internal Server Error`

## 3. `GET /users`

### 목적

전체 사용자 목록을 조회한다.

### 내부 SQL 변환

```sql
SELECT * FROM users;
```

### 성공 응답

- 상태 코드: `200 OK`

```json
{
  "ok": true,
  "rows": [
    {
      "id": 1,
      "name": "Alice",
      "age": 20
    }
  ],
  "row_count": 1
}
```

## 4. `GET /users/{id}`

### 목적

특정 사용자 한 건을 조회한다.

### 내부 SQL 변환

```sql
SELECT * FROM users WHERE id = 1;
```

### 성공 응답

- 상태 코드: `200 OK`

```json
{
  "ok": true,
  "rows": [
    {
      "id": 1,
      "name": "Alice",
      "age": 20
    }
  ],
  "row_count": 1
}
```

### 실패 응답

- `id`가 숫자가 아니거나 음수: `400 Bad Request`
- 해당 사용자가 없음: `404 Not Found`

## 에러 응답 형식

모든 실패 응답은 아래 구조를 기본으로 한다.

```json
{
  "ok": false,
  "error": "..."
}
```

## 구현 메모

- `POST /users` 응답에는 최소 구현 기준으로 `id`를 포함하지 않는다.
- TEAM7 엔진이 안정적으로 `inserted_id`를 제공하더라도, 이번 MVP에서는 응답 단순성을 우선한다.
