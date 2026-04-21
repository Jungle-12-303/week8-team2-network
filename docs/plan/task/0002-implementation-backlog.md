# 구현 백로그 정리함

## 목적

`plan` 문서의 큰 흐름을 실제 실행 가능한 세부 카드로 내려받기 위한 백로그다.

## 범위

### 세부 카드로 나누기

이 백로그는 아래 카드들로 쪼개서 관리한다.

- `0003-project-bootstrap.md`
- `0004-server-entry-and-config.md`
- `0005-http-request-parser.md`
- `0006-route-and-response-contract.md`
- `0007-sql-execution-adapter.md`
- `0008-thread-pool-and-job-queue.md`
- `0009-database-locking-and-shutdown.md`
- `0010-unit-test-cases.md`
- `0011-api-test-scenarios.md`
- `0012-concurrency-test-scenarios.md`
- `0013-demo-and-readme-checklist.md`
- `0014-error-message-standardization.md`
- `0015-validation-and-observed-output.md`

## 완료 기준

- 각 항목이 독립 카드로 더 쪼개질 수 있음
- 구현 순서와 테스트 순서가 보임
- 누락된 영역이 있으면 표시함

## 테스트/검증

- `docs/plan/06-implementation-order.md`와 대조함
- `docs/plan/07-test-and-quality-plan.md`와 대조함

## 의존성

- `docs/plan/00-plan-manager.md`
- `docs/plan/06-implementation-order.md`
- `docs/plan/07-test-and-quality-plan.md`

## 메모

- 이 문서는 최종 작업표가 아니라 출발점이다.
- 실제로는 위 항목을 더 잘게 나눠서 카드별로 관리하는 편이 좋다.
