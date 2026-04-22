# 테스트 스크립트 묶음 안내

이 폴더는 실제 테스트를 수행하는 스크립트의 본체를 담는다.

## SQL

- [단위 테스트](sql/unit-tests.sh)

## HTTP

- [스모크 테스트](http/smoke-test.sh)
- [통합 테스트](http/integration-test.sh)
- [프로토콜 경계값 테스트](http/protocol-edge-cases.sh)
- [수동 쿼리 확인](http/manual-query.sh)

## 동시성

- [rwlock 스트레스 테스트](concurrency/rwlock-stress-test.sh)

## 실행 순서 추천

테스트 기록표를 채울 때는 아래 순서가 가장 읽기 쉽다.

1. `sh scripts/tests/sql/unit-tests.sh`
2. `sh scripts/tests/http/smoke-test.sh 8080`
3. `sh scripts/tests/http/integration-test.sh`
4. `sh scripts/tests/http/protocol-edge-cases.sh`
5. `sh scripts/tests/http/manual-query.sh 8080`
6. `sh scripts/tests/concurrency/rwlock-stress-test.sh`

## 참고

루트의 `scripts/*.sh` 파일은 기존 명령 호환을 위한 래퍼다.  
새로 작성할 때는 이 `tests/` 하위 스크립트를 기준으로 보면 된다.
