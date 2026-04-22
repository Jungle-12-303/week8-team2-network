# 테스트 스크립트 묶음 안내

이 문서는 `scripts/tests/` 아래에 있는 실제 테스트 실행 파일을 빠르게 찾기 위한 안내문이다.

## SQL

- `sql/unit-tests.sh`
  - SQL 파서, B+Tree, 조건 조회, 결과 구조를 검증한다.

## HTTP

- `http/smoke-test.sh`
  - INSERT와 SELECT의 기본 동작을 확인한다.
- `http/integration-test.sh`
  - Docker 또는 로컬 서버 기준으로 HTTP 통합 동작을 확인한다.
- `http/protocol-edge-cases.sh`
  - 빈 body, Content-Length 누락/불일치, malformed request line을 확인한다.
- `http/timeout-test.sh`
  - 느린 요청이 타임아웃으로 정리되는지 확인한다.
- `http/manual-query.sh`
  - 서버에 직접 SQL을 입력해서 응답을 눈으로 확인한다.

## 동시성

- `concurrency/rwlock-stress-test.sh`
  - 버킷 락 기반 동시성 동작과 종료 처리를 확인한다.

## 권장 실행 순서

1. `sh scripts/tests/sql/unit-tests.sh`
2. `sh scripts/tests/http/smoke-test.sh 8080`
3. `sh scripts/tests/http/integration-test.sh`
4. `sh scripts/tests/http/protocol-edge-cases.sh`
5. `sh scripts/tests/http/timeout-test.sh`
6. `sh scripts/tests/http/manual-query.sh 8080`
7. `sh scripts/tests/concurrency/rwlock-stress-test.sh`

## 참고

- 루트의 `scripts/*.sh` 파일은 기존 호환용 래퍼다.
- 새로 문서를 볼 때는 `tests/` 하위 스크립트를 기준으로 보면 가장 헷갈리지 않는다.
