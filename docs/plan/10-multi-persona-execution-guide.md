# 10. Multi Persona Execution Guide

## Why We Need This

If we want to implement the plan folder in parallel, one agent thread is usually not enough.

The best pattern for this repo is not "many agents doing the same thing".
It is "a small set of specialized personas with clear ownership and one integrator keeping the system coherent".

## Recommended Persona Count

For this project, the sweet spot is:

- 1 coordinator persona
- 5 execution personas

So, 6 total personas is a good target.

If the work is small, use fewer.
If the scope grows a lot, you can temporarily split one persona into two, but do that only when the ownership boundaries are clear.

## Recommended Personas

### 1. Coordinator / Integrator

This is the control plane.

Personality:

- Calm
- Strict about interfaces
- Good at sequencing work
- Good at conflict resolution

Responsibilities:

- Own the single source of truth
- Assign task cards
- Prevent file overlap
- Merge outputs from other personas
- Resolve interface conflicts
- Keep the plan updated

This persona should not be the main writer for feature code unless it is also filling a gap temporarily.

### 2. HTTP / API Persona

This persona owns request/response behavior.

Personality:

- Precise
- Contract-focused
- Defensive about edge cases

Responsibilities:

- HTTP parsing
- Route handling
- Status codes
- Response format
- Error mapping at the API boundary

Best when working on:

- `docs/plan/task/0005-http-request-parser.md`
- `docs/plan/task/0006-route-and-response-contract.md`
- part of `docs/plan/task/0014-error-message-standardization.md`

### 3. DB / SQL Persona

This persona owns the bridge to the existing SQL engine.

Personality:

- Careful
- Interface-driven
- Low tolerance for breaking existing behavior

Responsibilities:

- `sql_execute()` integration
- Result mapping
- SQL error handling
- INSERT/SELECT contract clarity

Best when working on:

- `docs/plan/task/0007-sql-execution-adapter.md`
- part of `docs/plan/task/0014-error-message-standardization.md`

### 4. Concurrency / Runtime Persona

This persona owns runtime behavior under load.

Personality:

- Systems-minded
- Paranoid in a good way
- Obsessed with shutdown and locking correctness

Responsibilities:

- Thread pool
- Job queue
- Locking
- Graceful shutdown
- Queue full behavior

Best when working on:

- `docs/plan/task/0008-thread-pool-and-job-queue.md`
- `docs/plan/task/0009-database-locking-and-shutdown.md`
- part of `docs/plan/task/0012-concurrency-test-scenarios.md`

### 5. Testing / QA Persona

This persona proves the system works.

Personality:

- Skeptical
- Methodical
- Good at finding missing cases

Responsibilities:

- Unit test planning
- API scenario planning
- Concurrency scenario planning
- Regression checks
- Validation against expected output

Best when working on:

- `docs/plan/task/0010-unit-test-cases.md`
- `docs/plan/task/0011-api-test-scenarios.md`
- `docs/plan/task/0012-concurrency-test-scenarios.md`
- `docs/plan/task/0015-validation-and-observed-output.md`

### 6. Docs / Demo Persona

This persona makes the work visible.

Personality:

- Clear
- Concise
- User-facing

Responsibilities:

- README updates
- Demo flow
- Example commands
- Result summaries

Best when working on:

- `docs/plan/task/0013-demo-and-readme-checklist.md`
- parts of `docs/plan/task/0015-validation-and-observed-output.md`

## How They Should Work Together

### Rule 1. One Owner Per File

No two personas should edit the same file at the same time.

If two personas need the same area, the coordinator must split the file or sequence the work.

### Rule 2. Shared Contract First

Before implementation starts, the coordinator should lock:

- API shape
- Error codes
- Result format
- Queue behavior
- Shutdown behavior

That prevents parallel work from drifting apart.

### Rule 3. Use Task Cards As The Boundary

Each persona should receive task cards, not vague goals.

The card should include:

- Purpose
- Scope
- Done criteria
- Validation steps
- Dependencies

### Rule 4. Handoffs Must Be Written

Every persona should leave a short handoff note:

- What changed
- What remains
- What assumptions were made
- What the next persona should not break

### Rule 5. QA Runs Against Finished Interfaces

Testing can start early, but the QA persona should prefer stable contracts.

Good pattern:

- API persona defines contract
- DB and concurrency personas implement the internals
- QA persona validates the contract and catches regressions

## Parallel Execution Model

The best parallel pattern for this repo is:

1. Coordinator creates the task split
2. API persona works on request/response
3. DB persona works on SQL integration
4. Concurrency persona works on runtime structure
5. Testing persona prepares and validates test cases
6. Docs persona prepares README and demo material
7. Coordinator merges and resolves any conflicts

This works because the file ownership is naturally different.

## Persona Docs

Each persona has a dedicated guide in [`personas/README.md`](personas/README.md).

Use that folder for the detailed operating rules, ownership matrix, and handoff protocol.

## What Not To Do

- Do not let multiple personas rewrite the same contract file
- Do not start implementation before the API and error format are agreed
- Do not let QA become the only place where requirements are discovered
- Do not have the coordinator act as a bottleneck for every tiny change
- Do not create more personas than the project can actually feed

## My Recommendation For This Repo

Start with 6 personas total.

If you want a simpler setup, collapse them to 4:

- Coordinator / Integrator
- API + Docs
- DB + Concurrency
- Testing / QA

But if you want the cleanest parallel flow, 6 is better for this codebase because:

- API work is distinct from DB work
- concurrency has enough complexity to deserve a separate owner
- tests and docs are both important enough to be separate

## Bottom Line

For this project, the optimal setup is:

- 1 coordinator
- 5 specialist personas

That gives us enough parallelism without turning coordination into overhead.
