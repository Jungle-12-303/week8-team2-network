# HTTP 타임아웃 테스트 안내

이 문서는 느린 요청이 서버를 오래 묶지 않고 타임아웃으로 정리되는지 확인하는 테스트를 설명한다.

## 목적

- 느리게 들어오는 요청이 일정 시간 뒤 `408 Request Timeout`으로 정리되는지 확인한다.
- 타임아웃이 발생해도 다른 정상 요청이 계속 처리되는지 확인한다.
- 대기 중인 워커가 풀려서 서버가 멈추지 않는지 확인한다.

## 실행 스크립트

- `sh scripts/tests/http/timeout-test.sh`
- 호환용 루트 래퍼: `sh scripts/http_timeout_test.sh`

## 실행 전제

- 스크립트가 서버를 자동으로 띄운다.
- Docker가 가능하면 Docker로 실행하고, 아니면 로컬 `make db_server` / `./db_server` 방식으로 전환한다.
- `python3`가 있으면 그걸 쓰고, 없으면 `python`을 시도한다.

## 스크립트가 확인하는 내용

1. 느린 요청 4개를 먼저 보낸다.
2. 서버가 응답을 늦추는 동안 정상적인 `INSERT` 요청 1개를 추가로 보낸다.
3. 느린 요청은 `408 Request Timeout`이 되는지 확인한다.
4. 정상 요청은 `200 OK`와 `"ok":true`를 받는지 확인한다.
5. 정상 요청이 타임아웃 해제 이후 실제로 완료되는지 확인한다.

## 수동 확인 방법

이 테스트는 기본적으로 자동화 스크립트로 보는 것이 맞다. 직접 눈으로 확인하고 싶다면 아래처럼 실행한다.

```bash
sh scripts/tests/http/timeout-test.sh
```

실행이 끝나면 다음과 같은 결과를 기대한다.

```text
HTTP timeout test passed.
```

## 참고 파일

- [`scripts/tests/http/timeout-test.sh`](../../../scripts/tests/http/timeout-test.sh)
- [`docs/test/demo/tests/http-integration-test.md`](http-integration-test.md)
- [`docs/test/demo/tests/http-protocol-edge-cases.md`](http-protocol-edge-cases.md)
