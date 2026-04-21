# 03. API Contract

## 코드 주석 원칙

API 계약을 구현하는 코드에는 HTTP method/path 검증, `Content-Length` 처리, body 추출, SQL 오류 응답 변환 기준을 한국어 주석으로 설명합니다. 클라이언트 요청 형식 오류와 SQL 실행 오류를 왜 다르게 다루는지도 코드 근처에 남깁니다.

이 문서는 외부 클라이언트가 사용할 API 형태를 정의합니다. MVP는 HTTP 기반 단일 SQL 실행 API입니다.

## 기본 엔드포인트

```http
POST /query HTTP/1.1
Content-Type: text/plain

SELECT * FROM users;
```

또는 JSON body를 쓰고 싶다면 다음 형태를 확장 후보로 둡니다.

```json
{
  "sql": "SELECT * FROM users;"
}
```

MVP 기본값은 `text/plain` body에 SQL 문자열을 직접 담는 방식입니다. C에서 파싱이 단순하고 기존 `sql_execute()`와 바로 연결하기 쉽기 때문입니다.

## 성공 응답

INSERT 성공 예시:

```http
HTTP/1.1 200 OK
Content-Type: application/json

{
  "ok": true,
  "action": "insert",
  "inserted_id": 1,
  "row_count": 1
}
```

SELECT 성공 예시:

```http
HTTP/1.1 200 OK
Content-Type: application/json

{
  "ok": true,
  "action": "select",
  "row_count": 2,
  "rows": [
    { "id": 1, "name": "Alice", "age": 20 },
    { "id": 2, "name": "Bob", "age": 30 }
  ]
}
```

조회 결과가 없는 경우:

```json
{
  "ok": true,
  "action": "select",
  "row_count": 0,
  "rows": []
}
```

## SQL 오류 응답

SQL 파싱 또는 실행 오류는 HTTP 요청 자체는 정상 처리된 것으로 보고 `200 OK`와 JSON body로 반환합니다.

```json
{
  "ok": false,
  "status": "syntax_error",
  "error_code": 1064,
  "sql_state": "42000",
  "message": "ERROR 1064 (42000): ..."
}
```

HTTP 요청 형식이 잘못된 경우에는 HTTP status code로 실패를 표현합니다.

## 지원 SQL

기존 SQL 처리기가 지원하는 문법만 API로 노출합니다.

```sql
INSERT INTO users VALUES ('Alice', 20);
SELECT * FROM users;
SELECT * FROM users WHERE id = 1;
SELECT * FROM users WHERE id >= 10;
SELECT * FROM users WHERE name = 'Alice';
SELECT * FROM users WHERE age > 20;
```

## HTTP 제약

- `POST /query`만 지원합니다.
- request body 최대 길이를 정합니다. MVP 기본값은 4096 bytes입니다.
- chunked transfer encoding은 지원하지 않습니다.
- keep-alive는 지원하지 않고 요청 하나 처리 후 연결을 닫습니다.
- 응답에는 `Connection: close`를 포함합니다.

## curl 데모 예시

```bash
curl -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "INSERT INTO users VALUES ('Alice', 20);"
```

```bash
curl -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "SELECT * FROM users;"
```
