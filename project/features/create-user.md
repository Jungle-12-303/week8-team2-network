# Create User

## 목적

`POST /users`는 외부 클라이언트가 새 사용자를 생성하고, 내부적으로는 TEAM7 엔진의 `users` 테이블에 새 레코드를 추가하기 위한 기능이다.

이 기능은 API 서버가 REST 요청을 받아 내부 SQL 실행으로 연결하는 대표 경로다.

## 사용자 관점 기대 동작

- 클라이언트가 이름과 나이를 담은 JSON을 보내면 서버가 새 사용자를 생성한다.
- 입력 형식이 올바르고 내부 DB 처리에 성공하면 `201 Created`를 반환한다.
- 입력이 잘못되면 `400 Bad Request`를 반환한다.
- 내부 SQL 실행 또는 엔진 처리에 실패하면 `500 Internal Server Error`를 반환한다.

## 요청 입력

### HTTP Request

- Method: `POST`
- Path: `/users`
- Content-Type: JSON

### Request Body

```json
{
  "name": "Alice",
  "age": 20
}
```

### 필수 입력 필드

- `name`: 사용자 이름 문자열
- `age`: 사용자 나이 정수

### 입력 검증

- `name` 필드는 반드시 존재해야 한다.
- `age` 필드는 반드시 존재해야 한다.
- `name`은 빈 문자열이면 안 된다.
- `age`는 정수여야 한다.
- 최소 구현에서는 서버와 TEAM7 엔진이 처리 가능한 범위를 넘는 과도한 입력은 `400`으로 거절할 수 있다.

## 응답 출력

### 성공 응답

- Status: `201 Created`
- Body:

```json
{
  "ok": true,
  "message": "created"
}
```

참고:

- TEAM7 엔진 수정 범위와 구현 방식에 따라 생성된 `id`를 안정적으로 얻을 수 있으면 응답에 포함할 수 있다.
- 다만 MVP acceptance criteria는 `id` 반환을 필수로 요구하지 않는다.

### 입력 오류 응답

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

- `POST /users`가 JSON body를 받아 새 사용자를 생성할 수 있다.
- 유효한 요청에 대해 서버는 `201 Created`와 JSON 응답을 반환한다.
- `name` 또는 `age`가 누락된 요청은 `400 Bad Request`를 반환한다.
- JSON 형식이 깨졌거나 서버가 해석할 수 없는 body는 `400 Bad Request`를 반환한다.
- 내부 SQL 변환과 TEAM7 엔진 실행이 성공하면 실제로 이후 조회에서 생성 결과를 확인할 수 있다.
- 이 기능은 thread pool 환경에서 worker가 처리 가능한 작업 단위로 동작해야 한다.

## 구현해야 할 항목

- `/users`에 대한 `POST` 라우팅
- request body 읽기와 JSON 파싱
- 입력 검증
- REST 요청을 내부 SQL `INSERT`로 변환
- TEAM7 엔진 실행 결과를 HTTP 응답으로 매핑
- 생성 성공 및 실패 응답 형식 통일

## 제외 범위

- 복수 테이블에 대한 사용자 생성
- 중복 이름 검증 정책
- 입력 문자열 정규화
- 인증, 권한 확인
- 배치 생성
- 임의 SQL을 body로 직접 받는 기능
