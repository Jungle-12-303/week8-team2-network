# 09. Plan And Task Management Guide

## Why This Guide Exists

`plan` documents are best for direction, scope, ordering, and testing strategy.
`task` documents are best for the tiny implementation cards we can actually execute.

Keeping them separate makes the work easier to track.

## Recommended Structure

```text
docs/
  plan/
    00-plan-manager.md
    01-requirements-and-scope.md
    02-server-architecture.md
    03-api-contract.md
    04-thread-pool-and-concurrency.md
    05-db-engine-integration.md
    06-implementation-order.md
    07-test-and-quality-plan.md
    08-demo-and-readme-plan.md
    09-plan-and-task-management-guide.md
    10-multi-persona-execution-guide.md
    11-implementation-policies.md
    12-graceful-shutdown-strategy.md
    generated/
      README.md
      http-integration-test-plan.md
    task/
      README.md
      template.md
      0001-*.md
      0002-*.md
```

## How To Use `plan`

Use `plan` to answer:

- What are we building?
- What is in scope?
- What should we decide first?
- What is the implementation order?
- What tests are required?
- What should the README and demo cover?

`plan` is the decision layer, not the execution layer.

## How To Use `task`

Use `task` for the smallest actionable work units.

Good task cards should:

- Be readable on one page
- Have clear completion criteria
- Include test or validation steps
- Show dependencies
- Stay small enough to finish without losing focus

## Title Rule

Prefer titles ending with `~함`.

- Good: `라우터 응답 포맷 정리함`
- Good: `단위 테스트 케이스 추가함`
- Good: `에러 메시지 표준화함`
- Less good: `라우터 응답 포맷 정리해야 함`

Short titles are easier to manage and easier to treat like task cards.

## Is Testing Already In `plan`?

Yes.

`docs/plan/07-test-and-quality-plan.md` already covers:

- Unit tests
- API tests
- Concurrency tests
- Error handling checks
- Pre-demo checklist

So testing is already planned. The better split is:

- `plan` holds the testing strategy
- `task` holds the concrete test cards

## Recommended Workflow

1. Set scope and order in `plan`
2. Split implementation into small `task` cards
3. Write completion criteria for each card
4. Mark cards done while implementing
5. Update test planning when coverage changes

## Extra Ideas

### Decision Log

If an important choice keeps showing up, consider a separate `docs/decisions/` area for it.

Examples:

- Why use HTTP
- Why choose a specific response shape
- Why the implementation order is what it is
- Why a test was added or delayed

### Milestone Cards

Grouping tasks by milestone makes progress easier to read.

Examples:

- `M1` foundation
- `M2` API contract
- `M3` concurrency
- `M4` tests
- `M5` demo and docs

### Multi Persona Execution

If we want to split implementation across multiple Codex personas, use the operating guide in [`10-multi-persona-execution-guide.md`](10-multi-persona-execution-guide.md).

That guide defines the recommended persona count, ownership rules, and handoff behavior.

Detailed persona responsibilities live in [`personas/README.md`](personas/README.md).

Implementation policies, including socket I/O, SQL error mapping, shutdown, queue behavior, and build rules, live in [`11-implementation-policies.md`](11-implementation-policies.md).

Graceful shutdown choices are compared in [`12-graceful-shutdown-strategy.md`](12-graceful-shutdown-strategy.md).

### Generated Plans

If we create a new plan during development and want to keep it separate from the canonical plan set, place it under [`generated/`](generated/).

Use that folder for:

- newly created test plans
- temporary or working plans
- plans produced while iterating on implementation

Keep the canonical top-level `plan` files for project-wide direction, and keep generated plans isolated so they do not blur the baseline roadmap.

## Bottom Line

- `plan` = direction and design rules
- `task` = small implementation cards
- testing strategy stays in `plan`, while test execution items live in `task`

That keeps the docs clean and the work easier to manage.
