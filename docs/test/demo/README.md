# 테스트 안내

이 문서는 현재 테스트 폴더와 실제 실행 스크립트를 함께 묶어서 보는 안내서다.

기본 원칙은 다음과 같다.

- `scripts/tests/` 아래의 스크립트는 개별 검증용 정식 테스트다.
- 루트의 `scripts/*.sh` 파일은 데모나 호환성을 위한 진입점이다.
- `docs/test/demo/tests/`는 기본 테스트 문서를 모아 둔 허브다.
- `docs/test/backlog/`, `docs/test/queue-503/`, `docs/test/queue-log/`는 최근 추가된 서버 운영/동시성 검증 문서다.

## 문서 목록

- [SQL 단위 테스트](tests/unit-tests.md)
- [HTTP 스모크 테스트](tests/http-smoke-test.md)
- [HTTP 통합 테스트](tests/http-integration-test.md)
- [HTTP 프로토콜 경계값 테스트](tests/http-protocol-edge-cases.md)
- [HTTP 타임아웃 테스트](tests/http-timeout-test.md)
- [수동 쿼리 확인](tests/manual-query.md)
- [버킷 락 동시성 테스트](tests/bucket-lock-test.md)

## 추가 테스트 문서

- [backlog 테스트](../backlog/README.md)
- [queue full / 503 테스트](../queue-503/README.md)
- [multi client / queue log 데모](../queue-log/README.md)

## 추천 실행 순서

### 기본 검증

| 순서 | 실행 명령 | 주로 확인하는 내용 | 실행 결과 |
| --- | --- | --- | --- |
| 1 | `sh scripts/tests/sql/unit-tests.sh` | SQL 내부 로직, 파서, 결과 구조 | PASS |
| 2 | `sh scripts/tests/http/smoke-test.sh 8080` | INSERT / SELECT 기본 응답 | PASS |
| 3 | `sh scripts/tests/http/integration-test.sh` | HTTP 통합 동작과 상태 코드 | PASS |
| 4 | `sh scripts/tests/http/protocol-edge-cases.sh` | 요청 형식 경계값 | PASS |
| 5 | `sh scripts/tests/http/timeout-test.sh` | 느린 요청의 타임아웃 처리 | PASS |
| 6 | `sh scripts/tests/concurrency/bucket-lock-stress-test.sh` | 버킷 락 동시성 및 종료 처리 | FAIL - 2분 내 종료하지 않아 timeout |
| 7 | `sh scripts/tests/http/manual-query.sh 8080` | 사람이 직접 SQL 결과를 확인 | PASS |

### 서버 부하 / 운영 검증

| 순서 | 실행 명령 | 주로 확인하는 내용 |
| --- | --- | --- |
| 1 | `sh scripts/backlog_test.sh` | `listen(backlog)`와 TCP 대기열 한계 |
| 2 | `sh scripts/queue_503_test.sh` | `thread_pool_submit()` 초과 시 503 응답 |
| 3 | `sh scripts/multi_client_demo.sh` | 다중 클라이언트, `DEBUG_QUEUE` 로그, queue 포화 흐름 |

## 검증 방식

| 방식 | 의미 | 대표 스크립트 |
| --- | --- | --- |
| 자동 스크립트 | 실행만 하면 결과를 바로 판정할 수 있는 경우 | `sql/unit-tests.sh`, `http/smoke-test.sh`, `http/integration-test.sh`, `http/protocol-edge-cases.sh`, `http/timeout-test.sh`, `concurrency/bucket-lock-stress-test.sh`, `backlog_test.sh`, `queue_503_test.sh`, `multi_client_demo.sh` |
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
| HTTP | 느린 요청 타임아웃 | 오래 걸리는 요청이 `408`로 정리되는지 | [scripts/tests/http/timeout-test.sh](../../../scripts/tests/http/timeout-test.sh) | [http-timeout-test.md](tests/http-timeout-test.md) | 미실행 | - | - | [docs/plan/07-test-and-quality-plan.md](../../../docs/plan/07-test-and-quality-plan.md) |
| HTTP | `GET /query` | `405 Method Not Allowed`가 오는지 | [scripts/tests/http/integration-test.sh](../../../scripts/tests/http/integration-test.sh) | [http-integration-test.md](tests/http-integration-test.md) | 미실행 | - | - | [docs/plan/07-test-and-quality-plan.md](../../../docs/plan/07-test-and-quality-plan.md) |
| HTTP | `POST /unknown` | `404 Not Found`가 오는지 | [scripts/tests/http/integration-test.sh](../../../scripts/tests/http/integration-test.sh) | [http-integration-test.md](tests/http-integration-test.md) | 미실행 | - | - | [docs/plan/07-test-and-quality-plan.md](../../../docs/plan/07-test-and-quality-plan.md) |
| HTTP | `413 Payload Too Large` | 과도한 body를 거부하는지 | [scripts/tests/http/integration-test.sh](../../../scripts/tests/http/integration-test.sh) | [http-integration-test.md](tests/http-integration-test.md) | 미실행 | - | - | [docs/plan/07-test-and-quality-plan.md](../../../docs/plan/07-test-and-quality-plan.md) |
| 수동 | `SELECT` 결과 가독성 | 응답이 사람이 보기 좋게 출력되는지 | [scripts/tests/http/manual-query.sh](../../../scripts/tests/http/manual-query.sh) | [manual-query.md](tests/manual-query.md) | 미실행 | - | - | [docs/plan/task/0015-validation-and-observed-output.md](../../../docs/plan/task/0015-validation-and-observed-output.md) |
| 동시성 | 여러 `SELECT` 동시 요청 | 읽기 경로가 병렬로 잘 처리되는지 | [scripts/tests/concurrency/bucket-lock-stress-test.sh](../../../scripts/tests/concurrency/bucket-lock-stress-test.sh) | [bucket-lock-test.md](tests/bucket-lock-test.md) | 미실행 | - | - | [docs/plan/task/0012-concurrency-test-scenarios.md](../../../docs/plan/task/0012-concurrency-test-scenarios.md) <br /> [docs/plan/04-thread-pool-and-concurrency.md](../../../docs/plan/04-thread-pool-and-concurrency.md) |
| 동시성 | 여러 `INSERT` 동시 요청 | 쓰기 경합 중에도 중복/누락이 없는지 | [scripts/tests/concurrency/bucket-lock-stress-test.sh](../../../scripts/tests/concurrency/bucket-lock-stress-test.sh) | [bucket-lock-test.md](tests/bucket-lock-test.md) | 미실행 | - | - | [docs/plan/task/0012-concurrency-test-scenarios.md](../../../docs/plan/task/0012-concurrency-test-scenarios.md) <br /> [docs/plan/04-thread-pool-and-concurrency.md](../../../docs/plan/04-thread-pool-and-concurrency.md) |
| 동시성 | 교차 패턴 재현 | `SELECT`와 `INSERT`가 섞여도 순서와 결과가 유지되는지 | [scripts/tests/concurrency/bucket-lock-stress-test.sh](../../../scripts/tests/concurrency/bucket-lock-stress-test.sh) | [bucket-lock-test.md](tests/bucket-lock-test.md) | 미실행 | - | - | [docs/plan/task/0012-concurrency-test-scenarios.md](../../../docs/plan/task/0012-concurrency-test-scenarios.md) <br /> [docs/plan/04-thread-pool-and-concurrency.md](../../../docs/plan/04-thread-pool-and-concurrency.md) |
| 서버/운영 | TCP backlog 한계 | `listen(backlog)`가 실제 연결 수용에 미치는 영향 | [scripts/backlog_test.sh](../../../scripts/backlog_test.sh) | [backlog/README.md](../backlog/README.md) | 미실행 | - | - | [docs/plan/04-thread-pool-and-concurrency.md](../../../docs/plan/04-thread-pool-and-concurrency.md) |
| 서버/운영 | queue full / 503 | `thread_pool_submit()` 초과 시 503 응답 | [scripts/queue_503_test.sh](../../../scripts/queue_503_test.sh) | [queue-503/README.md](../queue-503/README.md) | 미실행 | - | - | [docs/plan/04-thread-pool-and-concurrency.md](../../../docs/plan/04-thread-pool-and-concurrency.md) |
| 서버/운영 | multi client / queue log | 다중 클라이언트와 `[QUEUE]` 로그 흐름 확인 | [scripts/multi_client_demo.sh](../../../scripts/multi_client_demo.sh) | [queue-log/README.md](../queue-log/README.md) | 미실행 | - | - | [docs/plan/04-thread-pool-and-concurrency.md](../../../docs/plan/04-thread-pool-and-concurrency.md) |

## 테스트 기록 규칙

- `규격 문서`는 항상 맨 마지막 열에 둔다.
- `실행 결과`는 실제 실행 전에는 `미실행`으로 남긴다.
- `실패 사유`는 테스트가 실패했을 때만 적는다.
- `해결 기록`은 어떤 수정으로 복구했는지 적는다.

## 참고

- `scripts/tests/` 아래 문서는 개별 검증 기준이다.
- 루트의 `scripts/*.sh` 파일은 이전 실행 방식과의 호환성, 혹은 데모 목적의 진입점이다.
- 새로운 테스트가 추가되면 먼저 `scripts/` 또는 `scripts/tests/`에 실행 스크립트를 넣고, 그다음 `docs/test/demo/tests/` 또는 `docs/test/*/README.md`에 안내 문서를 붙인다.
## Demo Scenario

- [demo-scenario.md](demo-scenario.md)
