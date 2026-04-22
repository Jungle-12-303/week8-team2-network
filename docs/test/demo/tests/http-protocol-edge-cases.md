# HTTP 프로토콜 경계값 확인

이 문서는 `scripts/tests/http/protocol-edge-cases.sh`로 HTTP 요청 형식이 깨졌을 때 서버가 어떻게 반응하는지 확인하는 방법을 설명합니다.

## 무엇을 테스트하나

이 스크립트는 curl로는 자연스럽게 만들기 어려운 요청을 직접 만들어 보냅니다.
raw 요청 전송에는 `python3`를 사용합니다.

- 빈 SQL body가 들어왔을 때의 응답
- `Content-Length` 헤더가 없을 때의 응답
- 요청 시작줄이 깨졌을 때의 응답
- `Content-Length`와 실제 body가 맞지 않을 때의 응답

즉, HTTP 파서가 잘못된 입력을 안전하게 거절하는지, 그리고 SQL 계층까지 내려가는 경우에는 어떤 에러 JSON이 나오는지 함께 확인합니다.

## 수동 실행 방법

프로젝트 루트에서 실행합니다.

```bash
sh scripts/tests/http/protocol-edge-cases.sh
```

또는 호환용 루트 스크립트를 쓸 수 있습니다.

```bash
sh scripts/http_protocol_edge_cases.sh
```

스크립트는 실행 환경에 따라 자동으로 모드를 고릅니다.

- Docker가 있고 데몬에 접근 가능하면 Docker 모드로 실행한다
- Docker를 쓸 수 없으면 `db_server`를 로컬에서 직접 띄워서 실행한다

## 기대 결과

정상적으로 통과하면 마지막에 아래 문구가 출력됩니다.

```text
HTTP protocol edge-case tests passed.
```

실행 중에는 케이스별 응답도 함께 볼 수 있습니다.

- `EMPTY BODY`
- `MISSING CONTENT-LENGTH`
- `MALFORMED REQUEST LINE`
- `CONTENT-LENGTH MISMATCH`

## 케이스별 기대 응답

### 1) 빈 body

- HTTP `200 OK`
- JSON 응답
- `status:"syntax_error"`
- `error_code:1064`
- `sql_state:"42000"`

### 2) Content-Length 누락

- HTTP `400 Bad Request`
- `status:"bad_request"`
- `message:"Missing Content-Length"`

### 3) malformed request line

- HTTP `400 Bad Request`
- `status:"bad_request"`
- `message:"Malformed HTTP request line"`

### 4) Content-Length 불일치

- HTTP `400 Bad Request`
- `status:"bad_request"`
- `message:"Incomplete request body"`

## 같이 보면 좋은 파일

- [scripts/tests/http/protocol-edge-cases.sh](../../../scripts/tests/http/protocol-edge-cases.sh)
- [http-integration-test.md](http-integration-test.md)
- [http-smoke-test.md](http-smoke-test.md)
- [docs/plan/07-test-and-quality-plan.md](../../../docs/plan/07-test-and-quality-plan.md)
- [server/http.c](../../../server/http.c)
- [server/server.c](../../../server/server.c)
