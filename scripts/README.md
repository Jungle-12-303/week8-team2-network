# 테스트 스크립트 안내

이 폴더는 프로젝트에서 반복적으로 돌리는 테스트 스크립트를 모아 둔 곳이다.

## 구조

- `tests/sql/`
  - SQL 엔진과 단위 테스트를 확인하는 스크립트
- `tests/http/`
  - HTTP 스모크, 통합, 프로토콜 경계값, 수동 쿼리 확인 스크립트
- `tests/concurrency/`
  - `rwlock`과 동시성 동작을 확인하는 스크립트

## 권장 실행 순서

1. `sh scripts/tests/sql/unit-tests.sh`
2. `sh scripts/tests/http/smoke-test.sh 8080`
3. `sh scripts/tests/http/integration-test.sh`
4. `sh scripts/tests/http/protocol-edge-cases.sh`
5. `sh scripts/tests/http/manual-query.sh 8080`
6. `sh scripts/tests/concurrency/rwlock-stress-test.sh`

## 호환성

기존에 쓰던 루트 스크립트도 그대로 사용할 수 있다.

- `scripts/smoke_test.sh`
- `scripts/unit-tests.sh`
- `scripts/http_integration_test.sh`
- `scripts/http_protocol_edge_cases.sh`
- `scripts/manual_query.sh`
- `scripts/rwlock_stress_test.sh`

이 루트 스크립트들은 현재 `tests/` 하위의 실제 구현을 호출하는 래퍼다.
