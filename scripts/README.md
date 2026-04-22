# 테스트 스크립트 안내

이 폴더는 저장소 전체에서 공통으로 쓰는 테스트 실행 스크립트를 모아 둔 곳이다.

## 구조

- `tests/sql/`
  - SQL 단위 테스트를 실행한다.
- `tests/http/`
  - HTTP 스모크, 통합, 프로토콜 경계값, 타임아웃, 수동 확인 테스트를 실행한다.
- `tests/concurrency/`
  - 버킷 락과 동시성 동작을 확인하는 테스트를 실행한다.

## 권장 실행 순서

1. `sh scripts/tests/sql/unit-tests.sh`
2. `sh scripts/tests/http/smoke-test.sh 8080`
3. `sh scripts/tests/http/integration-test.sh`
4. `sh scripts/tests/http/protocol-edge-cases.sh`
5. `sh scripts/tests/http/timeout-test.sh`
6. `sh scripts/tests/http/manual-query.sh 8080`
7. `sh scripts/tests/concurrency/rwlock-stress-test.sh`

## 기존 루트 스크립트

예전부터 쓰던 루트 래퍼 스크립트도 같이 남겨 두었다.

- `scripts/smoke_test.sh`
- `scripts/unit-tests.sh`
- `scripts/http_integration_test.sh`
- `scripts/http_protocol_edge_cases.sh`
- `scripts/http_timeout_test.sh`
- `scripts/manual_query.sh`
- `scripts/rwlock_stress_test.sh`

실제 테스트 코드는 이제 `tests/` 하위 스크립트를 기준으로 보는 것이 가장 정확하다.
