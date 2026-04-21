# Ownership Matrix

## 목적

어떤 영역을 누가 책임지는지 빠르게 보기 위한 표다.

## 기본 매핑

| 영역 | 주 담당 |
| --- | --- |
| HTTP parsing | HTTP / API |
| route and response contract | HTTP / API |
| SQL execution adapter | DB / SQL |
| result mapping | DB / SQL |
| thread pool | Concurrency / Runtime |
| job queue | Concurrency / Runtime |
| locking | Concurrency / Runtime |
| unit tests | Testing / QA |
| API scenarios | Testing / QA |
| concurrency scenarios | Testing / QA |
| README | Docs / Demo |
| demo flow | Docs / Demo |
| overall sequencing | Coordinator / Integrator |

## 병렬 작업 기준

- API와 DB는 동시에 진행 가능하다
- Concurrency는 API와 병렬 가능하지만 계약은 먼저 확인해야 한다
- QA는 구현과 함께 시작할 수 있지만 안정 계약이 필요하다
- Docs는 구현 결과가 나온 뒤 빠르게 따라붙는다

## 금지 규칙

- 같은 파일을 두 사람이 동시에 수정하지 않는다
- 계약이 없는 상태에서 구현을 확장하지 않는다
- QA가 마지막에만 존재하면 안 된다
