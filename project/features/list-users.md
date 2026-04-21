# List Users

## 목적

`GET /users`는 현재 `users` 테이블에 저장된 전체 사용자 목록을 외부 클라이언트가 조회할 수 있게 하는 기능이다.

이 기능은 TEAM7 엔진의 전체 조회 결과를 서버가 JSON 배열로 변환해 반환하는 역할을 한다.

## 사용자 관점 기대 동작

- 클라이언트가 `GET /users`를 호출하면 현재 저장된 사용자 목록을 JSON으로 받는다.
- 사용자가 하나 이상 있으면 전체 목록을 반환한다.
- 사용자가 없더라도 요청 자체는 성공이며, 빈 목록을 반환한다.
- 데이터 조회 요청이므로 서버 데이터 상태를 변경하지 않는다.

## 요청 입력

### HTTP Request

- Method: `GET`
- Path: `/users`
- Request Body: 없음

### 입력 검증

- request body는 사용하지 않는다.
- 최소 구현에서는 query parameter 기반 필터링을 지원하지 않는다.

## 응답 출력

### 성공 응답

- Status: `200 OK`
- Body 예시:

```json
{
  "ok": true,
  "rows": [
    {
      "id": 1,
      "name": "Alice",
      "age": 20
    },
    {
      "id": 2,
      "name": "Bob",
      "age": 31
    }
  ],
  "row_count": 2
}
```

### 빈 결과 응답

- Status: `200 OK`
- Body 예시:

```json
{
  "ok": true,
  "rows": [],
  "row_count": 0
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

- `GET /users` 요청에 대해 서버가 `200 OK`를 반환한다.
- 성공 응답은 JSON 형식이며 `rows` 배열과 `row_count`를 포함한다.
- 데이터가 없으면 빈 배열과 `row_count: 0`을 반환한다.
- 데이터가 있으면 TEAM7 엔진 조회 결과를 누락 없이 JSON으로 반환한다.
- 이 요청은 데이터 상태를 변경하지 않는다.
- 다중 조회 요청이 들어와도 서버가 read 경로로 안정적으로 처리할 수 있다.

## 구현해야 할 항목

- `/users`에 대한 `GET` 라우팅
- 내부 SQL `SELECT * FROM users;` 변환
- TEAM7 조회 결과를 서버용 결과 구조체로 변환
- 결과 목록을 JSON 배열로 직렬화
- 빈 결과 처리

## 제외 범위

- `name`, `age` query parameter 필터링
- 정렬 기준 선택
- 페이지네이션
- 부분 필드 조회
- 복수 테이블 join
