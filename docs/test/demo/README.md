# 테스트 안내

이 폴더는 실제 테스트 스크립트와 그 설명 문서를 1:1로 연결해 둔 곳이다.

## 문서 목록

- [SQL 단위 테스트](tests/unit-tests.md)
- [HTTP 스모크 테스트](tests/http-smoke-test.md)
- [HTTP 통합 테스트](tests/http-integration-test.md)
- [HTTP 프로토콜 경계값 테스트](tests/http-protocol-edge-cases.md)
- [HTTP 타임아웃 테스트](tests/http-timeout-test.md)
- [수동 쿼리 확인](tests/manual-query.md)
- [버킷 락 동시성 테스트](tests/rwlock-test.md)

## 추천 실행 순서

| 순서 | 실행 명령 | 주로 확인하는 내용 |
| --- | --- | --- |
| 1 | `sh scripts/tests/sql/unit-tests.sh` | SQL 내부 로직과 결과 구조 |
| 2 | `sh scripts/tests/http/smoke-test.sh 8080` | INSERT / SELECT 기본 응답 |
| 3 | `sh scripts/tests/http/integration-test.sh` | HTTP 통합 동작과 상태 코드 |
| 4 | `sh scripts/tests/http/protocol-edge-cases.sh` | 요청 형식 경계값 |
| 5 | `sh scripts/tests/http/timeout-test.sh` | 느린 요청의 타임아웃 처리 |
| 6 | `sh scripts/tests/http/manual-query.sh 8080` | 수동 SQL 입력과 응답 가독성 |
| 7 | `sh scripts/tests/concurrency/rwlock-stress-test.sh` | 버킷 락 동시성 및 종료 처리 |

## 검증 방식

| 방식 | 의미 | 대표 스크립트 |
| --- | --- | --- |
| 자동 스크립트 | 실행만 하면 결과를 바로 판정할 수 있는 경우 | `sql/unit-tests.sh`, `http/smoke-test.sh`, `http/integration-test.sh`, `http/protocol-edge-cases.sh`, `http/timeout-test.sh`, `concurrency/rwlock-stress-test.sh` |
| 수동 확인 | 사람이 직접 결과를 눈으로 확인하는 경우 | `http/manual-query.sh` |

## 테스트 기록표

| 분류 | 테스트 항목 | 확인 포인트 | 실행 스크립트 | 테스트 안내 | 실행 결과 | 실패 사유 | 해결 기록 | 규격 문서 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| SQL | 단위 테스트 전체 | SQL 파서, B+Tree, 조건 조회, 결과 구조가 유지되는지 | [scripts/tests/sql/unit-tests.sh](../../../scripts/tests/sql/unit-tests.sh) | [unit-tests.md](tests/unit-tests.md) | 미실행 | - | - | [docs/plan/task/0010-unit-test-cases.md](../../../docs/plan/task/0010-unit-test-cases.md) |
| HTTP | `POST /query` INSERT | insert 응답이 정상인지 | [scripts/tests/http/smoke-test.sh](../../../scripts/tests/http/smoke-test.sh) | [http-smoke-test.md](tests/http-smoke-test.md) | 미실행 | - | - | [docs/plan/03-api-contract.md](../../../docs/plan/03-api-contract.md) <br /> [docs/plan/07-test-and-quality-plan.md](../../../docs/plan/07-test-and-quality-plan.md) |
| HTTP | `POST /query` SELECT | select 응답이 정상인지 | [scripts/tests/http/smoke-test.sh](../../../scripts/tests/http/smoke-test.sh) | [http-smoke-test.md](tests/http-smoke-test.md) | 미실행 | - | - | [docs/plan/03-api-contract.md](../../../docs/plan/03-api-contract.md) <br /> [docs/plan/07-test-and-quality-plan.md](../../../docs/plan/07-test-and-quality-plan.md) |
| HTTP | JSON 응답 형식 | `ok`, `action`, `rows`가 유지되는지 | [scripts/tests/http/smoke-test.sh](../../../scripts/tests/http/smoke-test.sh) | [http-smoke-test.md](tests/http-smoke-test.md) | 미실행 | - | - | [docs/plan/03-api-contract.md](../../../docs/plan/03-api-contract.md) <br /> [docs/plan/07-test-and-quality-plan.md](../../../docs/plan/07-test-and-quality-plan.md) |
| HTTP | 빈 body `POST /query` | 빈 body를 안전하게 처리하는지 | [scripts/tests/http/protocol-edge-cases.sh](../../../scripts/tests/http/protocol-edge-cases.sh) | [http-protocol-edge-cases.md](tests/http-protocol-edge-cases.md) | 미실행 | - | - | [docs/plan/03-api-contract.md](../../../docs/plan/03-api-contract.md) <br /> [docs/plan/07-test-and-quality-plan.md](../../../docs/plan/07-test-and-quality-plan.md) |
| HTTP | `Content-Length` 누락 | 헤더 누락을 안전하게 거부하는지 | [scripts/tests/http/protocol-edge-cases.sh](../../../scripts/tests/http/protocol-edge-cases.sh) | [http-protocol-edge-cases.md](tests/http-protocol-edge-cases.md) | 미실행 | - | - | [docs/plan/03-api-contract.md](../../../docs/plan/03-api-contract.md) <br /> [docs/plan/07-test-and-quality-plan.md](../../../docs/plan/07-test-and-quality-plan.md) |
| HTTP | malformed request line | 요청 시작줄이 깨져도 서버가 죽지 않는지 | [scripts/tests/http/protocol-edge-cases.sh](../../../scripts/tests/http/protocol-edge-cases.sh) | [http-protocol-edge-cases.md](tests/http-protocol-edge-cases.md) | 미실행 | - | - | [docs/plan/07-test-and-quality-plan.md](../../../docs/plan/07-test-and-quality-plan.md) |
| HTTP | `Content-Length` 불일치 | body 길이가 맞지 않아도 안전하게 막는지 | [scripts/tests/http/protocol-edge-cases.sh](../../../scripts/tests/http/protocol-edge-cases.sh) | [http-protocol-edge-cases.md](tests/http-protocol-edge-cases.md) | 미실행 | - | - | [docs/plan/07-test-and-quality-plan.md](../../../docs/plan/07-test-and-quality-plan.md) |
| HTTP | 느린 요청 타임아웃 | 오래 걸리는 요청이 408로 정리되는지 | [scripts/tests/http/timeout-test.sh](../../../scripts/tests/http/timeout-test.sh) | [http-timeout-test.md](tests/http-timeout-test.md) | 미실행 | - | - | [docs/plan/07-test-and-quality-plan.md](../../../docs/plan/07-test-and-quality-plan.md) |
| HTTP | `GET /query` | `405 Method Not Allowed`가 오는지 | [scripts/tests/http/integration-test.sh](../../../scripts/tests/http/integration-test.sh) | [http-integration-test.md](tests/http-integration-test.md) | 미실행 | - | - | [docs/plan/07-test-and-quality-plan.md](../../../docs/plan/07-test-and-quality-plan.md) |
| HTTP | `POST /unknown` | `404 Not Found`가 오는지 | [scripts/tests/http/integration-test.sh](../../../scripts/tests/http/integration-test.sh) | [http-integration-test.md](tests/http-integration-test.md) | 미실행 | - | - | [docs/plan/07-test-and-quality-plan.md](../../../docs/plan/07-test-and-quality-plan.md) |
| HTTP | `413 Payload Too Large` | 과도한 body를 거부하는지 | [scripts/tests/http/integration-test.sh](../../../scripts/tests/http/integration-test.sh) | [http-integration-test.md](tests/http-integration-test.md) | 미실행 | - | - | [docs/plan/07-test-and-quality-plan.md](../../../docs/plan/07-test-and-quality-plan.md) |
| 수동 | `SELECT` 결과 가독성 | 응답이 사람이 보기 좋게 출력되는지 | [scripts/tests/http/manual-query.sh](../../../scripts/tests/http/manual-query.sh) | [manual-query.md](tests/manual-query.md) | 미실행 | - | - | [docs/plan/task/0015-validation-and-observed-output.md](../../../docs/plan/task/0015-validation-and-observed-output.md) |
| 동시성 | 여러 `SELECT` 동시 요청 | 읽기 경로가 병렬로 잘 처리되는지 | [scripts/tests/concurrency/rwlock-stress-test.sh](../../../scripts/tests/concurrency/rwlock-stress-test.sh) | [rwlock-test.md](tests/rwlock-test.md) | 미실행 | - | - | [docs/plan/task/0012-concurrency-test-scenarios.md](../../../docs/plan/task/0012-concurrency-test-scenarios.md) <br /> [docs/plan/04-thread-pool-and-concurrency.md](../../../docs/plan/04-thread-pool-and-concurrency.md) |
| 동시성 | 여러 `INSERT` 동시 요청 | 쓰기 경합 중에도 중복/누락이 없는지 | [scripts/tests/concurrency/rwlock-stress-test.sh](../../../scripts/tests/concurrency/rwlock-stress-test.sh) | [rwlock-test.md](tests/rwlock-test.md) | 미실행 | - | - | [docs/plan/task/0012-concurrency-test-scenarios.md](../../../docs/plan/task/0012-concurrency-test-scenarios.md) <br /> [docs/plan/04-thread-pool-and-concurrency.md](../../../docs/plan/04-thread-pool-and-concurrency.md) |
| 동시성 | 교차 패턴 재현 | `SELECT`와 `INSERT`가 섞여도 순서와 결과가 유지되는지 | [scripts/tests/concurrency/rwlock-stress-test.sh](../../../scripts/tests/concurrency/rwlock-stress-test.sh) | [rwlock-test.md](tests/rwlock-test.md) | 미실행 | - | - | [docs/plan/task/0012-concurrency-test-scenarios.md](../../../docs/plan/task/0012-concurrency-test-scenarios.md) <br /> [docs/plan/04-thread-pool-and-concurrency.md](../../../docs/plan/04-thread-pool-and-concurrency.md) |

## 테스트 기록 규칙

- `규격 문서`는 항상 맨 마지막 열에 둔다.
- `실행 결과`는 실제 실행 전에는 `미실행`으로 남긴다.
- `실패 사유`는 테스트가 실패했을 때만 적는다.
- `해결 기록`은 어떤 수정으로 복구했는지 적는다.

## 참고

- 현재 문서 세트는 실제 스크립트와 맞춰 두었다.
- 추가 스크립트가 필요한 경우가 생기면, 먼저 `scripts/tests/` 아래에 스크립트를 만들고 그다음 `docs/test/demo/tests/`에 안내 문서를 추가한다.
