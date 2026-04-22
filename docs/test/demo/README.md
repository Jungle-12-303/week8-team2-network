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
- [수동 쿼리 확인](tests/manual-query.md)
- [rwlock 수동 테스트](tests/rwlock-test.md)

## 기록 규칙

| 항목 | 의미 |
| --- | --- |
| `규격 문서` | 테스트해야 하는 기준과 기대 결과를 적은 문서 |
| `실행 결과` | `미실행`, `통과`, `실패` 중 하나 |
| `실패 사유` | 왜 통과하지 못했는지 적는 칸 |
| `해결 기록` | 어떤 수정으로 해결했는지 적는 칸 |

## 테스트 기록표

### 1) 단위 테스트와 SQL 계층 엣지 케이스

| 범주 | 테스트 상황 | 확인 포인트 | 규격 문서 | 실행 결과 | 실패 사유 | 해결 기록 |
| --- | --- | --- | --- | --- | --- | --- |
| 단위 | 빈 B+Tree 검색 | 데이터가 없어도 검색이 안전하게 끝나는지 | [unit-tests.md](tests/unit-tests.md) <br /> [docs/plan/task/0010-unit-test-cases.md](../../../docs/plan/task/0010-unit-test-cases.md) | 미실행 | - | - |
| 단위 | 단일 key insert/search | 하나의 row를 넣고 바로 찾을 수 있는지 | [unit-tests.md](tests/unit-tests.md) <br /> [docs/plan/task/0010-unit-test-cases.md](../../../docs/plan/task/0010-unit-test-cases.md) | 미실행 | - | - |
| 단위 | duplicate key 거부 | 같은 key가 중복 삽입되지 않는지 | [unit-tests.md](tests/unit-tests.md) <br /> [docs/plan/task/0010-unit-test-cases.md](../../../docs/plan/task/0010-unit-test-cases.md) | 미실행 | - | - |
| 단위 | leaf split 후 검색 유지 | leaf split 뒤에도 기존 데이터 검색이 유지되는지 | [unit-tests.md](tests/unit-tests.md) <br /> [docs/plan/task/0010-unit-test-cases.md](../../../docs/plan/task/0010-unit-test-cases.md) | 미실행 | - | - |
| 단위 | internal split 후 검색 유지 | internal split 뒤에도 탐색 경로가 유지되는지 | [unit-tests.md](tests/unit-tests.md) <br /> [docs/plan/task/0010-unit-test-cases.md](../../../docs/plan/task/0010-unit-test-cases.md) | 미실행 | - | - |
| 단위 | leaf next 링크 유지 | range scan이 leaf link를 통해 계속 이어지는지 | [unit-tests.md](tests/unit-tests.md) <br /> [docs/plan/task/0010-unit-test-cases.md](../../../docs/plan/task/0010-unit-test-cases.md) | 미실행 | - | - |
| 단위 | table auto increment | 삽입 시 id가 자동 증가하는지 | [unit-tests.md](tests/unit-tests.md) <br /> [docs/plan/task/0010-unit-test-cases.md](../../../docs/plan/task/0010-unit-test-cases.md) | 미실행 | - | - |
| 단위 | `id` 기반 검색 | primary key 조회가 정확한지 | [unit-tests.md](tests/unit-tests.md) <br /> [docs/plan/task/0010-unit-test-cases.md](../../../docs/plan/task/0010-unit-test-cases.md) | 미실행 | - | - |
| 단위 | `name` 기반 선형 검색 | 문자열 조건 조회가 동작하는지 | [unit-tests.md](tests/unit-tests.md) <br /> [docs/plan/task/0010-unit-test-cases.md](../../../docs/plan/task/0010-unit-test-cases.md) | 미실행 | - | - |
| 단위 | `age` 기반 선형 검색 | 숫자 조건 조회가 동작하는지 | [unit-tests.md](tests/unit-tests.md) <br /> [docs/plan/task/0010-unit-test-cases.md](../../../docs/plan/task/0010-unit-test-cases.md) | 미실행 | - | - |
| 단위 | 조건 검색 결과 수집 | 조건에 맞는 row들이 빠짐없이 모이는지 | [unit-tests.md](tests/unit-tests.md) <br /> [docs/plan/task/0010-unit-test-cases.md](../../../docs/plan/task/0010-unit-test-cases.md) | 미실행 | - | - |
| 단위 | `SQLResult` 구조 검증 | 반환 구조가 HTTP 직렬화 전에 깨지지 않는지 | [unit-tests.md](tests/unit-tests.md) <br /> [docs/plan/task/0010-unit-test-cases.md](../../../docs/plan/task/0010-unit-test-cases.md) | 미실행 | - | - |
| 단위 | SQL 오류 메시지 | 잘못된 SQL이 에러로 떨어지는지 | [unit-tests.md](tests/unit-tests.md) <br /> [docs/plan/task/0010-unit-test-cases.md](../../../docs/plan/task/0010-unit-test-cases.md) | 미실행 | - | - |

### 2) HTTP 기능과 엣지 케이스

| 범주 | 테스트 상황 | 확인 포인트 | 규격 문서 | 실행 결과 | 실패 사유 | 해결 기록 |
| --- | --- | --- | --- | --- | --- | --- |
| HTTP | `POST /query` INSERT | insert가 정상 응답을 주는지 | [http-smoke-test.md](tests/http-smoke-test.md) <br /> [http-integration-test.md](tests/http-integration-test.md) <br /> [scripts/smoke_test.sh](../../../scripts/smoke_test.sh) | 미실행 | - | - |
| HTTP | `POST /query` SELECT | select가 정상 응답을 주는지 | [http-smoke-test.md](tests/http-smoke-test.md) <br /> [http-integration-test.md](tests/http-integration-test.md) <br /> [scripts/smoke_test.sh](../../../scripts/smoke_test.sh) | 미실행 | - | - |
| HTTP | JSON 응답 형식 | `ok`, `action`, `rows` 등 응답 키가 깨지지 않는지 | [http-smoke-test.md](tests/http-smoke-test.md) <br /> [docs/plan/07-test-and-quality-plan.md](../../../docs/plan/07-test-and-quality-plan.md) | 미실행 | - | - |
| HTTP | `GET /query` | `405 Method Not Allowed`가 나오는지 | [http-integration-test.md](tests/http-integration-test.md) <br /> [scripts/http_integration_test.sh](../../../scripts/http_integration_test.sh) | 미실행 | - | - |
| HTTP | `POST /unknown` | `404 Not Found`가 나오는지 | [http-integration-test.md](tests/http-integration-test.md) <br /> [scripts/http_integration_test.sh](../../../scripts/http_integration_test.sh) | 미실행 | - | - |
| HTTP | 빈 body `POST /query` | body가 없을 때 `400` 계열로 안전하게 응답하는지 | [docs/plan/07-test-and-quality-plan.md](../../../docs/plan/07-test-and-quality-plan.md) | 미실행 | - | - |
| HTTP | malformed request line | 요청 시작줄이 깨졌을 때 서버가 죽지 않는지 | [docs/plan/07-test-and-quality-plan.md](../../../docs/plan/07-test-and-quality-plan.md) | 미실행 | - | - |
| HTTP | `Content-Length` 불일치 | body 길이와 헤더가 다를 때 안전하게 거절하는지 | [docs/plan/07-test-and-quality-plan.md](../../../docs/plan/07-test-and-quality-plan.md) | 미실행 | - | - |
| HTTP | `413 Payload Too Large` | 너무 큰 body를 거절하는지 | [http-integration-test.md](tests/http-integration-test.md) <br /> [scripts/http_integration_test.sh](../../../scripts/http_integration_test.sh) | 미실행 | - | - |
| HTTP | `SELECT` 결과 길이 | 조회 결과가 과도하게 길어져도 깨지지 않는지 | [manual-query.md](tests/manual-query.md) <br /> [docs/plan/task/0015-validation-and-observed-output.md](../../../docs/plan/task/0015-validation-and-observed-output.md) | 미실행 | - | - |

### 3) 수동 쿼리와 관측 출력

| 범주 | 테스트 상황 | 확인 포인트 | 규격 문서 | 실행 결과 | 실패 사유 | 해결 기록 |
| --- | --- | --- | --- | --- | --- | --- |
| 수동 | `INSERT INTO users VALUES ('Alice', 20);` | 수동 입력이 정상적으로 먹히는지 | [manual-query.md](tests/manual-query.md) | 미실행 | - | - |
| 수동 | `INSERT INTO users VALUES ('Bob', 30);` | 두 번째 insert도 정상인지 | [manual-query.md](tests/manual-query.md) | 미실행 | - | - |
| 수동 | `INSERT INTO users VALUES ('Carol', 25);` | 세 번째 insert도 정상인지 | [manual-query.md](tests/manual-query.md) | 미실행 | - | - |
| 수동 | `SELECT * FROM users;` | 전체 조회 결과가 읽히는지 | [manual-query.md](tests/manual-query.md) | 미실행 | - | - |
| 수동 | `SELECT * FROM users WHERE id = 1;` | id 단건 조회가 되는지 | [manual-query.md](tests/manual-query.md) | 미실행 | - | - |
| 수동 | `SELECT * FROM users WHERE name = 'Bob';` | 문자열 조건이 되는지 | [manual-query.md](tests/manual-query.md) | 미실행 | - | - |
| 수동 | `SELECT * FROM users WHERE age = 20;` | 동등 조건이 되는지 | [manual-query.md](tests/manual-query.md) | 미실행 | - | - |
| 수동 | `SELECT * FROM users WHERE age > 20;` | 대소 비교가 되는지 | [manual-query.md](tests/manual-query.md) | 미실행 | - | - |
| 수동 | `SELECT * FROM users WHERE age <= 20;` | 비교 연산이 유지되는지 | [manual-query.md](tests/manual-query.md) | 미실행 | - | - |

### 4) 동시성, 병렬성, 종료 처리

| 범주 | 테스트 상황 | 확인 포인트 | 규격 문서 | 실행 결과 | 실패 사유 | 해결 기록 |
| --- | --- | --- | --- | --- | --- | --- |
| 동시성 | 여러 `SELECT` 동시 요청 | 읽기끼리 서로 막지 않는지 | [docs/plan/task/0012-concurrency-test-scenarios.md](../../../docs/plan/task/0012-concurrency-test-scenarios.md) <br /> [docs/plan/04-thread-pool-and-concurrency.md](../../../docs/plan/04-thread-pool-and-concurrency.md) | 미실행 | - | - |
| 동시성 | 여러 `INSERT` 동시 요청 | 쓰기 경합 중에도 누락/중복이 없는지 | [docs/plan/task/0012-concurrency-test-scenarios.md](../../../docs/plan/task/0012-concurrency-test-scenarios.md) <br /> [docs/plan/04-thread-pool-and-concurrency.md](../../../docs/plan/04-thread-pool-and-concurrency.md) | 미실행 | - | - |
| 동시성 | `SELECT` + `INSERT` 혼합 | race condition이 드러나는지 | [docs/plan/task/0012-concurrency-test-scenarios.md](../../../docs/plan/task/0012-concurrency-test-scenarios.md) <br /> [docs/plan/04-thread-pool-and-concurrency.md](../../../docs/plan/04-thread-pool-and-concurrency.md) | 미실행 | - | - |
| 동시성 | `rwlock` read lock 분리 | 여러 읽기가 공유 락을 타는지 | [rwlock-test.md](tests/rwlock-test.md) <br /> [docs/plan/13-pthread-rwlock-strategy.md](../../../docs/plan/13-pthread-rwlock-strategy.md) <br /> [docs/plan/14-rwlock-test-plan.md](../../../docs/plan/14-rwlock-test-plan.md) | 미실행 | - | - |
| 동시성 | `rwlock` write lock 배타성 | 쓰기가 독점 락을 타는지 | [rwlock-test.md](tests/rwlock-test.md) <br /> [docs/plan/13-pthread-rwlock-strategy.md](../../../docs/plan/13-pthread-rwlock-strategy.md) <br /> [docs/plan/14-rwlock-test-plan.md](../../../docs/plan/14-rwlock-test-plan.md) | 미실행 | - | - |
| 동시성 | queue full | bounded queue가 가득 찼을 때 `503`이 나오는지 | [docs/plan/task/0012-concurrency-test-scenarios.md](../../../docs/plan/task/0012-concurrency-test-scenarios.md) <br /> [docs/plan/04-thread-pool-and-concurrency.md](../../../docs/plan/04-thread-pool-and-concurrency.md) | 미실행 | - | - |
| 종료 | graceful shutdown 중 요청 처리 | 종료 신호 이후 새 요청을 받지 않는지 | [docs/plan/12-graceful-shutdown-strategy.md](../../../docs/plan/12-graceful-shutdown-strategy.md) <br /> [docs/plan/task/0012-concurrency-test-scenarios.md](../../../docs/plan/task/0012-concurrency-test-scenarios.md) | 미실행 | - | - |
| 종료 | worker 정리 | `pthread_join()`까지 안전하게 끝나는지 | [docs/plan/12-graceful-shutdown-strategy.md](../../../docs/plan/12-graceful-shutdown-strategy.md) | 미실행 | - | - |
| 종료 | hang 없음 | 종료 과정에서 deadlock이나 무한 대기가 없는지 | [docs/plan/07-test-and-quality-plan.md](../../../docs/plan/07-test-and-quality-plan.md) <br /> [docs/plan/12-graceful-shutdown-strategy.md](../../../docs/plan/12-graceful-shutdown-strategy.md) | 미실행 | - | - |

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
