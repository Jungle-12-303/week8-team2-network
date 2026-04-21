# 비판적 평가 반영 결과

이 문서는 [`REVIEW-비판적-평가.md`](REVIEW-비판적-평가.md)의 지적사항을 현재 `plan` 문서에 어떻게 반영했는지 정리한 기록이다.

## 요약

비판적 평가에서 반복적으로 지적된 핵심은 다음 다섯 가지였다.

- HTTP 파싱이 너무 낙관적이었음
- SQL 오류 매핑이 불명확했음
- JSON 직렬화와 메모리 관리 책임이 흐렸음
- graceful shutdown이 빠져 있었음
- 동시성 테스트와 실패 경로가 부족했음

이 항목들은 현재 `docs/plan/03-api-contract.md`, `docs/plan/04-thread-pool-and-concurrency.md`, `docs/plan/05-db-engine-integration.md`, `docs/plan/06-implementation-order.md`, `docs/plan/07-test-and-quality-plan.md`, `docs/plan/08-demo-and-readme-plan.md`에 반영했다.

## 지적사항 대 반영 여부

| 지적사항 | 반영 여부 | 반영 위치 | 비고 |
| --- | --- | --- | --- |
| `POST /query`만 지원한다는 계약은 있으나, 실제 파서 규칙이 부족함 | 반영함 | [`docs/plan/03-api-contract.md`](/D:/03Dev/05Jungle/SuYo/Week8/week8-team2-network/docs/plan/03-api-contract.md) | request line, header, `Content-Length`, partial read, 잘못된 길이 값 기준을 추가함 |
| HTTP 오류와 SQL 오류의 경계가 불명확함 | 반영함 | [`docs/plan/03-api-contract.md`](/D:/03Dev/05Jungle/SuYo/Week8/week8-team2-network/docs/plan/03-api-contract.md), [`docs/plan/05-db-engine-integration.md`](/D:/03Dev/05Jungle/SuYo/Week8/week8-team2-network/docs/plan/05-db-engine-integration.md) | HTTP는 status code, SQL은 기본적으로 `200 OK` + JSON 실패 응답으로 분리함 |
| `SQLResult` 필드와 JSON 직렬화 책임이 불명확함 | 반영함 | [`docs/plan/05-db-engine-integration.md`](/D:/03Dev/05Jungle/SuYo/Week8/week8-team2-network/docs/plan/05-db-engine-integration.md) | 결과 직렬화 책임을 API 계층으로 명시함 |
| JSON 버퍼/메모리 관리 전략이 부족함 | 반영함 | [`docs/plan/04-thread-pool-and-concurrency.md`](/D:/03Dev/05Jungle/SuYo/Week8/week8-team2-network/docs/plan/04-thread-pool-and-concurrency.md), [`docs/plan/05-db-engine-integration.md`](/D:/03Dev/05Jungle/SuYo/Week8/week8-team2-network/docs/plan/05-db-engine-integration.md) | DB lock 안에서의 직렬화와 실패 경로를 더 분명히 적음 |
| graceful shutdown이 문서에 거의 없었음 | 반영함 | [`docs/plan/04-thread-pool-and-concurrency.md`](/D:/03Dev/05Jungle/SuYo/Week8/week8-team2-network/docs/plan/04-thread-pool-and-concurrency.md), [`docs/plan/06-implementation-order.md`](/D:/03Dev/05Jungle/SuYo/Week8/week8-team2-network/docs/plan/06-implementation-order.md) | 종료 플래그, worker wake-up, join 정책을 추가함 |
| queue full 처리와 503 정책이 흐릿함 | 반영함 | [`docs/plan/04-thread-pool-and-concurrency.md`](/D:/03Dev/05Jungle/SuYo/Week8/week8-team2-network/docs/plan/04-thread-pool-and-concurrency.md), [`docs/plan/06-implementation-order.md`](/D:/03Dev/05Jungle/SuYo/Week8/week8-team2-network/docs/plan/06-implementation-order.md) | 503의 위치와 helper 분리를 명시함 |
| 단위 테스트는 있으나 HTTP 실패 케이스가 약함 | 반영함 | [`docs/plan/07-test-and-quality-plan.md`](/D:/03Dev/05Jungle/SuYo/Week8/week8-team2-network/docs/plan/07-test-and-quality-plan.md) | partial read, length mismatch, malformed request를 추가함 |
| 동시성 테스트가 정상 insert 위주였음 | 반영함 | [`docs/plan/07-test-and-quality-plan.md`](/D:/03Dev/05Jungle/SuYo/Week8/week8-team2-network/docs/plan/07-test-and-quality-plan.md) | queue full, 종료 신호, id 중복/깨짐 방지를 추가함 |
| 발표/README가 실패 경로와 shutdown을 충분히 보여주지 못함 | 반영함 | [`docs/plan/08-demo-and-readme-plan.md`](/D:/03Dev/05Jungle/SuYo/Week8/week8-team2-network/docs/plan/08-demo-and-readme-plan.md) | HTTP 오류와 graceful shutdown 설명을 추가함 |
| 구현 순서가 기능 나열에 가깝고 실패 경로의 설계 순서가 약함 | 반영함 | [`docs/plan/06-implementation-order.md`](/D:/03Dev/05Jungle/SuYo/Week8/week8-team2-network/docs/plan/06-implementation-order.md) | 파싱, 응답, thread pool, shutdown, 검증 순으로 보강함 |

## 아직 남는 리스크

- 실제 `sql_processor`의 오류 코드/메시지 필드가 문서 가정과 다를 수 있다.
- `SQLResult`의 실제 구조에 따라 JSON 변환 코드의 메모리 전략은 구현 중 조정이 필요하다.
- graceful shutdown 방식은 sentinel job 방식과 종료 플래그 방식 중 최종 구현 시 하나를 확정해야 한다.
- HTTP 파서는 실제 소켓 읽기 구현에서 partial read, \r\n, body 누적 처리까지 직접 확인해야 한다.

## 결론

비판적 평가에서 지적된 핵심 약점은 현재 `plan` 문서에 대부분 반영됐다.
이제 남은 일은 문서대로 구현하면서 실제 `sql_processor` 구조와 소켓 I/O 제약을 기준으로 세부 구현을 확정하는 것이다.
