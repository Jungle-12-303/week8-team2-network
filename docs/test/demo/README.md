# 테스트 데모 안내

이 폴더는 서버를 직접 실행하면서 확인하는 테스트 문서를 모아 둡니다.

## 운영 방식

- `규격 문서`는 기대 동작만 적는 기준 문서로 둡니다.
- 실제 테스트를 돌린 뒤에는 이 README의 `실행 결과`를 갱신합니다.
- 테스트가 실패하면 `실패 사유`에 왜 통과하지 못했는지 적습니다.
- 이후 수정이 끝나면 `해결 기록`에 어떤 방식으로 고쳤는지 적습니다.
- 반복적으로 추적이 필요한 문제는 [해결기록 사용 가이드](../../Review/해결기록/README.md)를 따라 별도 문서로 분리합니다.

## 현재 문서

- [단위 테스트](tests/unit-tests.md)
- [HTTP 스모크 테스트](tests/http-smoke-test.md)
- [HTTP 통합 테스트](tests/http-integration-test.md)
- [HTTP 프로토콜 경계값 테스트](tests/http-protocol-edge-cases.md)
- [수동 쿼리 확인](tests/manual-query.md)
- [rwlock 수동 테스트](tests/rwlock-test.md)

## 스크립트 안내

- [테스트 스크립트 안내](../../../scripts/README.md)
- [테스트 스크립트 묶음 안내](../../../scripts/tests/README.md)

## 추천 실행 순서

테스트 기록표를 실제로 채울 때는 아래 순서로 도는 것이 가장 보기 쉽다.

| 순서 | 실행 명령 | 주로 채우는 기록표 |
| --- | --- | --- |
| 1 | `sh scripts/tests/sql/unit-tests.sh` | 단위 테스트와 SQL 계층 엣지 케이스 |
| 2 | `sh scripts/tests/http/smoke-test.sh 8080` | HTTP 기능과 기본 응답 형식 |
| 3 | `sh scripts/tests/http/integration-test.sh` | HTTP 엣지 케이스와 Docker / 로컬 자동 전환 |
| 4 | `sh scripts/tests/http/protocol-edge-cases.sh` | HTTP 프로토콜 경계값 |
| 5 | `sh scripts/tests/http/manual-query.sh 8080` | 수동 쿼리와 관측 출력 |
| 6 | `sh scripts/tests/concurrency/rwlock-stress-test.sh` | 동시성, 병렬성, 종료 처리 |

실행 순서는 보통 아래처럼 잡으면 좋다.

1. 단위 테스트로 SQL 엔진이 깨지지 않았는지 먼저 본다.
2. 스모크 테스트로 `INSERT` / `SELECT` 기본 흐름을 확인한다.
3. 통합 테스트로 `405`, `404`, `413` 같은 HTTP 경계값을 확인한다.
4. 프로토콜 경계값 테스트로 raw HTTP 이상 요청을 확인한다.
5. 수동 쿼리로 출력이 읽기 좋은지, 예시가 문서와 일치하는지 본다.
6. rwlock 테스트로 읽기/쓰기 락과 동시 요청을 확인한다.

## 검증 방식 분류

테스트 기록표의 각 행은 아래 셋 중 하나로 보면 가장 편하다.

| 분류 | 의미 | 대표 실행 방식 | 기록표에 반영할 때 |
| --- | --- | --- | --- |
| 자동 스크립트 | 한 번 실행하면 여러 세부 항목을 같이 확인하는 경우 | `sh scripts/tests/sql/unit-tests.sh`, `sh scripts/tests/http/smoke-test.sh 8080`, `sh scripts/tests/http/integration-test.sh`, `sh scripts/tests/http/protocol-edge-cases.sh`, `sh scripts/tests/concurrency/rwlock-stress-test.sh` | 해당 스크립트가 실제로 커버한 행의 `실행 결과`를 함께 갱신한다 |
| 수동 확인 | 사람이 명령을 넣고 응답을 직접 보는 경우 | `sh scripts/tests/http/manual-query.sh 8080` 또는 `curl` 직접 입력 | 눈으로 본 내용까지 `실행 결과`와 `해결 기록`에 적는다 |
| 추가 스크립트 필요 | 표에는 있지만 아직 자동화가 없는 경우 | 예: 아직 스크립트가 없는 새 HTTP 경계값이나 동시성 확장 케이스 | 먼저 새 스크립트나 one-off 명령을 만든 뒤 반영한다 |

## 기록 규칙

| 항목 | 의미 |
| --- | --- |
| `규격 문서` | 테스트해야 하는 기준과 기대 결과를 적은 문서 |
| `실행 결과` | `미실행`, `통과`, `실패` 중 하나 |
| `실패 사유` | 왜 통과하지 못했는지 적는 칸 |
| `해결 기록` | 어떤 수정으로 해결했는지 적는 칸 |

## 테스트 기록표

### 1) 단위 테스트와 SQL 계층 엣지 케이스

| 범주 | 테스트 상황 | 확인 포인트 | 실행 스크립트 | 테스트 안내 | 실행 결과 | 실패 사유 | 해결 기록 | 규격 문서 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 단위 | 빈 B+Tree 검색 | 데이터가 없어도 검색이 안전하게 끝나는지 | [scripts/tests/sql/unit-tests.sh](../../../scripts/tests/sql/unit-tests.sh) | [unit-tests.md](tests/unit-tests.md) | 미실행 | - | - | [docs/plan/task/0010-unit-test-cases.md](../../../docs/plan/task/0010-unit-test-cases.md) |
| 단위 | 단일 key insert/search | 하나의 row를 넣고 바로 찾을 수 있는지 | [scripts/tests/sql/unit-tests.sh](../../../scripts/tests/sql/unit-tests.sh) | [unit-tests.md](tests/unit-tests.md) | 미실행 | - | - | [docs/plan/task/0010-unit-test-cases.md](../../../docs/plan/task/0010-unit-test-cases.md) |
| 단위 | duplicate key 거부 | 같은 key가 중복 삽입되지 않는지 | [scripts/tests/sql/unit-tests.sh](../../../scripts/tests/sql/unit-tests.sh) | [unit-tests.md](tests/unit-tests.md) | 미실행 | - | - | [docs/plan/task/0010-unit-test-cases.md](../../../docs/plan/task/0010-unit-test-cases.md) |
| 단위 | leaf split 후 검색 유지 | leaf split 뒤에도 기존 데이터 검색이 유지되는지 | [scripts/tests/sql/unit-tests.sh](../../../scripts/tests/sql/unit-tests.sh) | [unit-tests.md](tests/unit-tests.md) | 미실행 | - | - | [docs/plan/task/0010-unit-test-cases.md](../../../docs/plan/task/0010-unit-test-cases.md) |
| 단위 | internal split 후 검색 유지 | internal split 뒤에도 탐색 경로가 유지되는지 | [scripts/tests/sql/unit-tests.sh](../../../scripts/tests/sql/unit-tests.sh) | [unit-tests.md](tests/unit-tests.md) | 미실행 | - | - | [docs/plan/task/0010-unit-test-cases.md](../../../docs/plan/task/0010-unit-test-cases.md) |
| 단위 | leaf next 링크 유지 | range scan이 leaf link를 통해 계속 이어지는지 | [scripts/tests/sql/unit-tests.sh](../../../scripts/tests/sql/unit-tests.sh) | [unit-tests.md](tests/unit-tests.md) | 미실행 | - | - | [docs/plan/task/0010-unit-test-cases.md](../../../docs/plan/task/0010-unit-test-cases.md) |
| 단위 | table auto increment | 삽입 시 id가 자동 증가하는지 | [scripts/tests/sql/unit-tests.sh](../../../scripts/tests/sql/unit-tests.sh) | [unit-tests.md](tests/unit-tests.md) | 미실행 | - | - | [docs/plan/task/0010-unit-test-cases.md](../../../docs/plan/task/0010-unit-test-cases.md) |
| 단위 | `id` 기반 검색 | primary key 조회가 정확한지 | [scripts/tests/sql/unit-tests.sh](../../../scripts/tests/sql/unit-tests.sh) | [unit-tests.md](tests/unit-tests.md) | 미실행 | - | - | [docs/plan/task/0010-unit-test-cases.md](../../../docs/plan/task/0010-unit-test-cases.md) |
| 단위 | `name` 기반 선형 검색 | 문자열 조건 조회가 동작하는지 | [scripts/tests/sql/unit-tests.sh](../../../scripts/tests/sql/unit-tests.sh) | [unit-tests.md](tests/unit-tests.md) | 미실행 | - | - | [docs/plan/task/0010-unit-test-cases.md](../../../docs/plan/task/0010-unit-test-cases.md) |
| 단위 | `age` 기반 선형 검색 | 숫자 조건 조회가 동작하는지 | [scripts/tests/sql/unit-tests.sh](../../../scripts/tests/sql/unit-tests.sh) | [unit-tests.md](tests/unit-tests.md) | 미실행 | - | - | [docs/plan/task/0010-unit-test-cases.md](../../../docs/plan/task/0010-unit-test-cases.md) |
| 단위 | 조건 검색 결과 수집 | 조건에 맞는 row들이 빠짐없이 모이는지 | [scripts/tests/sql/unit-tests.sh](../../../scripts/tests/sql/unit-tests.sh) | [unit-tests.md](tests/unit-tests.md) | 미실행 | - | - | [docs/plan/task/0010-unit-test-cases.md](../../../docs/plan/task/0010-unit-test-cases.md) |
| 단위 | `SQLResult` 구조 검증 | 반환 구조가 HTTP 직렬화 전에 깨지지 않는지 | [scripts/tests/sql/unit-tests.sh](../../../scripts/tests/sql/unit-tests.sh) | [unit-tests.md](tests/unit-tests.md) | 미실행 | - | - | [docs/plan/task/0010-unit-test-cases.md](../../../docs/plan/task/0010-unit-test-cases.md) |
| 단위 | SQL 오류 메시지 | 잘못된 SQL이 에러로 떨어지는지 | [scripts/tests/sql/unit-tests.sh](../../../scripts/tests/sql/unit-tests.sh) | [unit-tests.md](tests/unit-tests.md) | 미실행 | - | - | [docs/plan/task/0010-unit-test-cases.md](../../../docs/plan/task/0010-unit-test-cases.md) |

### 2) HTTP 기능과 엣지 케이스

| 범주 | 테스트 상황 | 확인 포인트 | 실행 스크립트 | 테스트 안내 | 실행 결과 | 실패 사유 | 해결 기록 | 규격 문서 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| HTTP | `POST /query` INSERT | insert가 정상 응답을 주는지 | [scripts/tests/http/smoke-test.sh](../../../scripts/tests/http/smoke-test.sh) | [http-smoke-test.md](tests/http-smoke-test.md) | 미실행 | - | - | [docs/plan/03-api-contract.md](../../../docs/plan/03-api-contract.md) <br /> [docs/plan/07-test-and-quality-plan.md](../../../docs/plan/07-test-and-quality-plan.md) |
| HTTP | `POST /query` SELECT | select가 정상 응답을 주는지 | [scripts/tests/http/smoke-test.sh](../../../scripts/tests/http/smoke-test.sh) | [http-smoke-test.md](tests/http-smoke-test.md) | 미실행 | - | - | [docs/plan/03-api-contract.md](../../../docs/plan/03-api-contract.md) <br /> [docs/plan/07-test-and-quality-plan.md](../../../docs/plan/07-test-and-quality-plan.md) |
| HTTP | JSON 응답 형식 | `ok`, `action`, `rows` 등 응답 키가 깨지지 않는지 | [scripts/tests/http/smoke-test.sh](../../../scripts/tests/http/smoke-test.sh) | [http-smoke-test.md](tests/http-smoke-test.md) | 미실행 | - | - | [docs/plan/03-api-contract.md](../../../docs/plan/03-api-contract.md) <br /> [docs/plan/07-test-and-quality-plan.md](../../../docs/plan/07-test-and-quality-plan.md) |
| HTTP | 빈 body `POST /query` | 빈 body가 안전하게 처리되는지 | [scripts/tests/http/protocol-edge-cases.sh](../../../scripts/tests/http/protocol-edge-cases.sh) | [http-protocol-edge-cases.md](tests/http-protocol-edge-cases.md) | 미실행 | - | - | [docs/plan/03-api-contract.md](../../../docs/plan/03-api-contract.md) <br /> [docs/plan/07-test-and-quality-plan.md](../../../docs/plan/07-test-and-quality-plan.md) |
| HTTP | `Content-Length` 누락 | 헤더가 없을 때도 안전하게 거절하는지 | [scripts/tests/http/protocol-edge-cases.sh](../../../scripts/tests/http/protocol-edge-cases.sh) | [http-protocol-edge-cases.md](tests/http-protocol-edge-cases.md) | 미실행 | - | - | [docs/plan/03-api-contract.md](../../../docs/plan/03-api-contract.md) <br /> [docs/plan/07-test-and-quality-plan.md](../../../docs/plan/07-test-and-quality-plan.md) |
| HTTP | malformed request line | 요청 시작줄이 깨졌을 때 서버가 죽지 않는지 | [scripts/tests/http/protocol-edge-cases.sh](../../../scripts/tests/http/protocol-edge-cases.sh) | [http-protocol-edge-cases.md](tests/http-protocol-edge-cases.md) | 미실행 | - | - | [docs/plan/07-test-and-quality-plan.md](../../../docs/plan/07-test-and-quality-plan.md) |
| HTTP | `Content-Length` 불일치 | body 길이와 헤더가 다를 때 안전하게 거절하는지 | [scripts/tests/http/protocol-edge-cases.sh](../../../scripts/tests/http/protocol-edge-cases.sh) | [http-protocol-edge-cases.md](tests/http-protocol-edge-cases.md) | 미실행 | - | - | [docs/plan/07-test-and-quality-plan.md](../../../docs/plan/07-test-and-quality-plan.md) |
| HTTP | `GET /query` | `405 Method Not Allowed`가 나오는지 | [scripts/tests/http/integration-test.sh](../../../scripts/tests/http/integration-test.sh) | [http-integration-test.md](tests/http-integration-test.md) | 미실행 | - | - | [docs/plan/07-test-and-quality-plan.md](../../../docs/plan/07-test-and-quality-plan.md) |
| HTTP | `POST /unknown` | `404 Not Found`가 나오는지 | [scripts/tests/http/integration-test.sh](../../../scripts/tests/http/integration-test.sh) | [http-integration-test.md](tests/http-integration-test.md) | 미실행 | - | - | [docs/plan/07-test-and-quality-plan.md](../../../docs/plan/07-test-and-quality-plan.md) |
| HTTP | `413 Payload Too Large` | 너무 큰 body를 거절하는지 | [scripts/tests/http/integration-test.sh](../../../scripts/tests/http/integration-test.sh) | [http-integration-test.md](tests/http-integration-test.md) | 미실행 | - | - | [docs/plan/07-test-and-quality-plan.md](../../../docs/plan/07-test-and-quality-plan.md) |
| HTTP | `SELECT` 결과 길이 | 조회 결과가 과도하게 길어져도 깨지지 않는지 | [scripts/tests/http/manual-query.sh](../../../scripts/tests/http/manual-query.sh) | [manual-query.md](tests/manual-query.md) | 미실행 | - | - | [docs/plan/task/0015-validation-and-observed-output.md](../../../docs/plan/task/0015-validation-and-observed-output.md) |

### 3) 수동 쿼리와 관측 출력

| 범주 | 테스트 상황 | 확인 포인트 | 실행 스크립트 | 테스트 안내 | 실행 결과 | 실패 사유 | 해결 기록 | 규격 문서 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 수동 | `INSERT INTO users VALUES ('Alice', 20);` | 수동 입력이 정상적으로 먹히는지 | [scripts/tests/http/manual-query.sh](../../../scripts/tests/http/manual-query.sh) | [manual-query.md](tests/manual-query.md) | 미실행 | - | - | [docs/plan/03-api-contract.md](../../../docs/plan/03-api-contract.md) |
| 수동 | `INSERT INTO users VALUES ('Bob', 30);` | 두 번째 insert도 정상인지 | [scripts/tests/http/manual-query.sh](../../../scripts/tests/http/manual-query.sh) | [manual-query.md](tests/manual-query.md) | 미실행 | - | - | [docs/plan/03-api-contract.md](../../../docs/plan/03-api-contract.md) |
| 수동 | `INSERT INTO users VALUES ('Carol', 25);` | 세 번째 insert도 정상인지 | [scripts/tests/http/manual-query.sh](../../../scripts/tests/http/manual-query.sh) | [manual-query.md](tests/manual-query.md) | 미실행 | - | - | [docs/plan/03-api-contract.md](../../../docs/plan/03-api-contract.md) |
| 수동 | `SELECT * FROM users;` | 전체 조회 결과가 읽히는지 | [scripts/tests/http/manual-query.sh](../../../scripts/tests/http/manual-query.sh) | [manual-query.md](tests/manual-query.md) | 미실행 | - | - | [docs/plan/03-api-contract.md](../../../docs/plan/03-api-contract.md) <br /> [docs/plan/07-test-and-quality-plan.md](../../../docs/plan/07-test-and-quality-plan.md) |
| 수동 | `SELECT * FROM users WHERE id = 1;` | id 단건 조회가 되는지 | [scripts/tests/http/manual-query.sh](../../../scripts/tests/http/manual-query.sh) | [manual-query.md](tests/manual-query.md) | 미실행 | - | - | [docs/plan/03-api-contract.md](../../../docs/plan/03-api-contract.md) |
| 수동 | `SELECT * FROM users WHERE name = 'Bob';` | 문자열 조건이 되는지 | [scripts/tests/http/manual-query.sh](../../../scripts/tests/http/manual-query.sh) | [manual-query.md](tests/manual-query.md) | 미실행 | - | - | [docs/plan/03-api-contract.md](../../../docs/plan/03-api-contract.md) |
| 수동 | `SELECT * FROM users WHERE age = 20;` | 동등 조건이 되는지 | [scripts/tests/http/manual-query.sh](../../../scripts/tests/http/manual-query.sh) | [manual-query.md](tests/manual-query.md) | 미실행 | - | - | [docs/plan/03-api-contract.md](../../../docs/plan/03-api-contract.md) |
| 수동 | `SELECT * FROM users WHERE age > 20;` | 대소 비교가 되는지 | [scripts/tests/http/manual-query.sh](../../../scripts/tests/http/manual-query.sh) | [manual-query.md](tests/manual-query.md) | 미실행 | - | - | [docs/plan/03-api-contract.md](../../../docs/plan/03-api-contract.md) |
| 수동 | `SELECT * FROM users WHERE age <= 20;` | 비교 연산이 유지되는지 | [scripts/tests/http/manual-query.sh](../../../scripts/tests/http/manual-query.sh) | [manual-query.md](tests/manual-query.md) | 미실행 | - | - | [docs/plan/03-api-contract.md](../../../docs/plan/03-api-contract.md) |

### 4) 동시성, 병렬성, 종료 처리

| 범주 | 테스트 상황 | 확인 포인트 | 실행 스크립트 | 테스트 안내 | 실행 결과 | 실패 사유 | 해결 기록 | 규격 문서 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 동시성 | 여러 `SELECT` 동시 요청 | 읽기끼리 서로 막지 않는지 | [scripts/tests/concurrency/rwlock-stress-test.sh](../../../scripts/tests/concurrency/rwlock-stress-test.sh) | [rwlock-test.md](tests/rwlock-test.md) | 미실행 | - | - | [docs/plan/task/0012-concurrency-test-scenarios.md](../../../docs/plan/task/0012-concurrency-test-scenarios.md) <br /> [docs/plan/04-thread-pool-and-concurrency.md](../../../docs/plan/04-thread-pool-and-concurrency.md) |
| 동시성 | 여러 `INSERT` 동시 요청 | 쓰기 경합 중에도 누락/중복이 없는지 | [scripts/tests/concurrency/rwlock-stress-test.sh](../../../scripts/tests/concurrency/rwlock-stress-test.sh) | [rwlock-test.md](tests/rwlock-test.md) | 미실행 | - | - | [docs/plan/task/0012-concurrency-test-scenarios.md](../../../docs/plan/task/0012-concurrency-test-scenarios.md) <br /> [docs/plan/04-thread-pool-and-concurrency.md](../../../docs/plan/04-thread-pool-and-concurrency.md) |
| 동시성 | 레이스 컨디션 재현 | `SELECT`와 `INSERT`가 섞일 때 결과 순서가 뒤섞이거나 누락되지 않는지 | [scripts/tests/concurrency/rwlock-stress-test.sh](../../../scripts/tests/concurrency/rwlock-stress-test.sh) | [rwlock-test.md](tests/rwlock-test.md) | 미실행 | - | - | [docs/plan/task/0012-concurrency-test-scenarios.md](../../../docs/plan/task/0012-concurrency-test-scenarios.md) <br /> [docs/plan/04-thread-pool-and-concurrency.md](../../../docs/plan/04-thread-pool-and-concurrency.md) |
| 동시성 | `rwlock` read lock 분리 | 여러 읽기가 공유 락을 타는지 | [scripts/tests/concurrency/rwlock-stress-test.sh](../../../scripts/tests/concurrency/rwlock-stress-test.sh) | [rwlock-test.md](tests/rwlock-test.md) | 미실행 | - | - | [docs/plan/13-pthread-rwlock-strategy.md](../../../docs/plan/13-pthread-rwlock-strategy.md) <br /> [docs/plan/14-rwlock-test-plan.md](../../../docs/plan/14-rwlock-test-plan.md) |
| 동시성 | `rwlock` write lock 배타성 | 쓰기가 독점 락을 타는지 | [scripts/tests/concurrency/rwlock-stress-test.sh](../../../scripts/tests/concurrency/rwlock-stress-test.sh) | [rwlock-test.md](tests/rwlock-test.md) | 미실행 | - | - | [docs/plan/13-pthread-rwlock-strategy.md](../../../docs/plan/13-pthread-rwlock-strategy.md) <br /> [docs/plan/14-rwlock-test-plan.md](../../../docs/plan/14-rwlock-test-plan.md) |
| 동시성 | queue full | bounded queue가 가득 찼을 때 `503`이 나오는지 | [scripts/tests/concurrency/rwlock-stress-test.sh](../../../scripts/tests/concurrency/rwlock-stress-test.sh) | [rwlock-test.md](tests/rwlock-test.md) | 미실행 | - | - | [docs/plan/task/0012-concurrency-test-scenarios.md](../../../docs/plan/task/0012-concurrency-test-scenarios.md) <br /> [docs/plan/04-thread-pool-and-concurrency.md](../../../docs/plan/04-thread-pool-and-concurrency.md) |
| 종료 | graceful shutdown 중 요청 처리 | 종료 신호 이후 새 요청을 받지 않는지 | [scripts/tests/concurrency/rwlock-stress-test.sh](../../../scripts/tests/concurrency/rwlock-stress-test.sh) | [rwlock-test.md](tests/rwlock-test.md) | 미실행 | - | - | [docs/plan/12-graceful-shutdown-strategy.md](../../../docs/plan/12-graceful-shutdown-strategy.md) <br /> [docs/plan/task/0012-concurrency-test-scenarios.md](../../../docs/plan/task/0012-concurrency-test-scenarios.md) |
| 종료 | worker 정리 | `pthread_join()`까지 안전하게 끝나는지 | [scripts/tests/concurrency/rwlock-stress-test.sh](../../../scripts/tests/concurrency/rwlock-stress-test.sh) | [rwlock-test.md](tests/rwlock-test.md) | 미실행 | - | - | [docs/plan/12-graceful-shutdown-strategy.md](../../../docs/plan/12-graceful-shutdown-strategy.md) |
| 종료 | hang 없음 | 종료 과정에서 deadlock이나 무한 대기가 없는지 | [scripts/tests/concurrency/rwlock-stress-test.sh](../../../scripts/tests/concurrency/rwlock-stress-test.sh) | [rwlock-test.md](tests/rwlock-test.md) | 미실행 | - | - | [docs/plan/07-test-and-quality-plan.md](../../../docs/plan/07-test-and-quality-plan.md) <br /> [docs/plan/12-graceful-shutdown-strategy.md](../../../docs/plan/12-graceful-shutdown-strategy.md) |

## 해석 기준

- `규격 문서`는 테스트가 기대하는 동작만 정의합니다.
- `실행 결과`는 실제 실행 후에만 `통과`로 바꿉니다.
- `실패 사유`는 테스트가 깨진 직접 원인을 적습니다.
- `해결 기록`은 어떤 수정으로 복구했는지 적습니다.

## 같이 보면 좋은 파일

- [docs/plan/07-test-and-quality-plan.md](../../../docs/plan/07-test-and-quality-plan.md)
- [docs/plan/04-thread-pool-and-concurrency.md](../../../docs/plan/04-thread-pool-and-concurrency.md)
- [docs/plan/12-graceful-shutdown-strategy.md](../../../docs/plan/12-graceful-shutdown-strategy.md)
- [docs/plan/task/0012-concurrency-test-scenarios.md](../../../docs/plan/task/0012-concurrency-test-scenarios.md)
- [docs/Review/해결기록/README.md](../../../docs/Review/해결기록/README.md)
