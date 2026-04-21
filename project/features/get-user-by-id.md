# Get User By Id

## 목적

`GET /users/{id}`는 특정 사용자 id를 기준으로 단건 조회를 수행하는 기능이다.

이 기능은 REST path parameter를 내부 SQL 조건 조회로 연결하고, 존재 여부에 따라 `200` 또는 `404`를 반환하는 최소 조회 API다.

## 사용자 관점 기대 동작

- 클라이언트가 특정 사용자 id를 포함한 경로를 호출하면 해당 사용자 한 건을 조회한다.
- 해당 id가 존재하면 `200 OK`와 사용자 정보를 반환한다.
- 해당 id가 없으면 `404 Not Found`를 반환한다.
- id 형식이 잘못되면 `400 Bad Request`를 반환한다.

## 요청 입력

### HTTP Request

- Method: `GET`
- Path: `/users/{id}`
- Request Body: 없음

### Path Parameter

- `id`: 조회 대상 사용자 식별자

예시:

```text
GET /users/1
```

### 입력 검증

- `id`는 정수여야 한다.
- 숫자가 아닌 값이나 빈 값은 잘못된 요청으로 처리한다.
- 최소 구현에서는 양의 정수 범위를 벗어나는 값은 `400`으로 거절할 수 있다.

## 응답 출력

### 성공 응답

- Status: `200 OK`
- Body 예시:

```json
{
  "ok": true,
  "row": {
    "id": 1,
    "name": "Alice",
    "age": 20
  }
}
```

### 존재하지 않는 id

- Status: `404 Not Found`
- Body 예시:

```json
{
  "ok": false,
  "error": "not_found"
}
```

### 잘못된 요청

- Status: `400 Bad Request`
- Body 예시:

```json
{
  "ok": false,
  "error": "invalid_request"
}
```

### 내부 처리 실패 응답

- Status: `500 Internal Server Error`
- Body 예시:

```json
{
  "ok": false,
  "error": "internal_error"
}
```

## Acceptance Criteria

- `GET /users/{id}` 요청에 대해 서버가 path에서 `id`를 추출할 수 있다.
- 유효한 `id`가 존재하면 `200 OK`와 단건 사용자 정보를 반환한다.
- 존재하지 않는 `id`는 `404 Not Found`를 반환한다.
- 숫자가 아닌 `id` 또는 해석 불가능한 경로는 `400 Bad Request`를 반환한다.
- 이 요청은 내부적으로 id 조건 조회를 수행하며 데이터 상태를 변경하지 않는다.
- 조회 결과가 정확히 0건 또는 1건으로 해석되도록 응답 형식을 유지한다.

## 구현해야 할 항목

- `/users/{id}` 경로 라우팅
- path parameter 파싱과 정수 검증
- 내부 SQL `SELECT * FROM users WHERE id = {id};` 변환
- TEAM7 조회 결과를 단건 응답 구조로 변환
- `200`, `404`, `400` 분기 처리

## 제외 범위

- 복수 조건 검색
- 이름 또는 나이 기반 path 조회
- soft delete 상태 처리
- 상세 프로필 확장 필드
- 권한별 조회 제어
