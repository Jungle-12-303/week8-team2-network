# 00. Plan Manager

## 목적

이 문서는 이 프로젝트의 최상위 출발점이다.

여기서는 다음을 먼저 고정한다.

- 무엇을 만들지
- 어떤 문서를 먼저 볼지
- 어떤 순서로 구현할지
- 어떤 기준으로 테스트할지
- 6명의 페르소나가 어떻게 나눠서 일할지

## 이 프로젝트의 기본 전제

- 기존 `sql_processor`는 유지하고 최대한 활용한다
- HTTP API 서버를 C로 구현한다
- 요청 처리, SQL 연동, 동시성 처리를 분리해서 본다
- 테스트와 발표 문서까지 구현 범위에 포함한다
- 구현은 6개 페르소나로 병렬 진행할 수 있도록 설계한다

## 전체 문서 흐름

1. [`01-requirements-and-scope.md`](01-requirements-and-scope.md)
2. [`02-server-architecture.md`](02-server-architecture.md)
3. [`03-api-contract.md`](03-api-contract.md)
4. [`04-thread-pool-and-concurrency.md`](04-thread-pool-and-concurrency.md)
5. [`05-db-engine-integration.md`](05-db-engine-integration.md)
6. [`06-implementation-order.md`](06-implementation-order.md)
7. [`07-test-and-quality-plan.md`](07-test-and-quality-plan.md)
8. [`08-demo-and-readme-plan.md`](08-demo-and-readme-plan.md)
9. [`09-plan-and-task-management-guide.md`](09-plan-and-task-management-guide.md)
10. [`10-multi-persona-execution-guide.md`](10-multi-persona-execution-guide.md)
11. [`11-implementation-policies.md`](11-implementation-policies.md)
12. [`12-graceful-shutdown-strategy.md`](12-graceful-shutdown-strategy.md)

## 구현 목표

- HTTP 기반 API 서버를 만든다
- `POST /query` 중심의 SQL 실행 경로를 만든다
- 기존 SQL 처리기와 연결한다
- 공유 DB 자원을 보호한다
- thread pool + bounded job queue로 동시성 요청을 처리한다
- 단위 테스트, API 테스트, 동시성 테스트를 준비한다
- README와 데모 문서를 실행 가능하게 만든다

## 6페르소나 운영 전제

이 프로젝트는 기본적으로 6명의 페르소나로 나눠서 구현하는 것을 권장한다.

- Coordinator / Integrator
- HTTP / API
- DB / SQL
- Concurrency / Runtime
- Testing / QA
- Docs / Demo

이렇게 나누는 이유는 역할 경계가 비교적 명확하기 때문이다.

- API는 요청/응답 계약을 본다
- DB는 SQL 엔진 연결을 본다
- Concurrency는 락과 큐를 본다
- QA는 검증을 본다
- Docs는 발표와 실행 방법을 본다

## 역할 매핑

| 영역 | 관련 문서 | 담당 페르소나 |
| --- | --- | --- |
| 서버 구조 | [02-server-architecture.md](02-server-architecture.md), [06-implementation-order.md](06-implementation-order.md) | Coordinator / Integrator, Concurrency / Runtime |
| HTTP 계약 | [03-api-contract.md](03-api-contract.md) | HTTP / API |
| 요청 파싱 | [03-api-contract.md](03-api-contract.md) | HTTP / API |
| 스레드 풀 / 큐 | [04-thread-pool-and-concurrency.md](04-thread-pool-and-concurrency.md) | Concurrency / Runtime |
| SQL 연동 | [05-db-engine-integration.md](05-db-engine-integration.md) | DB / SQL |
| 테스트 전략 | [07-test-and-quality-plan.md](07-test-and-quality-plan.md) | Testing / QA |
| 발표 / README | [08-demo-and-readme-plan.md](08-demo-and-readme-plan.md) | Docs / Demo |
| task 관리 | [09-plan-and-task-management-guide.md](09-plan-and-task-management-guide.md) | Coordinator / Integrator |
| 페르소나 운영 | [10-multi-persona-execution-guide.md](10-multi-persona-execution-guide.md), [personas/README.md](personas/README.md) | Coordinator / Integrator |

## 구현 시작 순서

1. `01-requirements-and-scope.md`로 범위를 확정한다
2. `02-server-architecture.md`로 구조를 고정한다
3. `03-api-contract.md`로 요청/응답 계약을 먼저 정한다
4. `04-thread-pool-and-concurrency.md`로 동시성 방식과 종료 방식을 정한다
5. `05-db-engine-integration.md`로 기존 SQL 엔진 연결 방식을 정한다
6. `06-implementation-order.md`를 따라 실제 구현 순서를 정한다
7. `07-test-and-quality-plan.md`으로 테스트를 붙인다
8. `08-demo-and-readme-plan.md`으로 발표와 README를 마무리한다
9. `09-plan-and-task-management-guide.md`로 task 카드를 운영한다
10. `10-multi-persona-execution-guide.md`와 `personas/README.md`로 6페르소나 작업을 돌린다

## 이 문서의 사용법

- 구현 시작 전에 가장 먼저 읽는다
- task를 만들기 전에 범위와 책임을 다시 확인한다
- 여러 페르소나가 동시에 일할 때 기준점으로 쓴다
- "지금 누가 무엇을 해야 하는가"가 흔들리면 다시 돌아온다

## 결론

이 프로젝트의 기본 실행 모델은 다음과 같다.

- 계획은 `plan`이 담당한다
- 세부 실행은 `task`가 담당한다
- 병렬 구현은 6페르소나 모델로 돌린다
- 최종 조율은 Coordinator / Integrator가 맡는다
- 실제 I/O, 오류 매핑, 종료, 큐, 빌드 규칙은 `11-implementation-policies.md`를 기준으로 고정한다
- graceful shutdown 방식은 `12-graceful-shutdown-strategy.md`에서 최종 확정한다
