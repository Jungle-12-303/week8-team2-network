# 비판적 평가2 반영 결과

이 문서는 [`REVIEW-비판적-평가2.md`](REVIEW-비판적-평가2.md)의 지적사항을 현재 `plan` 문서에 어떻게 반영했는지 정리한 기록이다.

## 반영 방침

이 평가에서는 하루 완성 가능성이나 예상 시간 같은 항목은 제외했다.
우리는 Codex 기반으로 병렬 실행하는 전제를 두고 있으므로, 이번 반영은 시간 추정보다 실행 계약과 구조적 리스크에 집중한다.

## 반영한 핵심

- task 카드에 담당 페르소나, 영향 파일, 선행/후속 작업, 인터페이스를 추가했다
- 페르소나 간 입력/출력 계약 문서를 추가했다
- socket I/O, SQL 오류 매핑, 종료, 큐, 빌드 정책을 별도 문서로 고정했다
- plan의 시작점과 task 운영 문서가 새 정책 문서를 참조하도록 연결했다

## 지적사항 대 반영 여부

| 지적사항 | 반영 여부 | 반영 위치 |
| --- | --- | --- |
| task 카드가 개념적이고 구현 명세가 약함 | 반영함 | [`docs/plan/task/template.md`](/D:/03Dev/05Jungle/SuYo/Week8/week8-team2-network/docs/plan/task/template.md), [`docs/plan/task/README.md`](/D:/03Dev/05Jungle/SuYo/Week8/week8-team2-network/docs/plan/task/README.md) |
| task마다 담당 페르소나와 파일 책임이 없음 | 반영함 | [`docs/plan/task/template.md`](/D:/03Dev/05Jungle/SuYo/Week8/week8-team2-network/docs/plan/task/template.md), [`docs/plan/personas/08-ownership-matrix.md`](/D:/03Dev/05Jungle/SuYo/Week8/week8-team2-network/docs/plan/personas/08-ownership-matrix.md) |
| 페르소나 간 인터페이스가 불분명함 | 반영함 | [`docs/plan/personas/09-persona-interface-contract.md`](/D:/03Dev/05Jungle/SuYo/Week8/week8-team2-network/docs/plan/personas/09-persona-interface-contract.md) |
| HTTP 파싱 규칙이 더 구체적이어야 함 | 반영함 | [`docs/plan/03-api-contract.md`](/D:/03Dev/05Jungle/SuYo/Week8/week8-team2-network/docs/plan/03-api-contract.md) |
| SQL 오류 매핑과 직렬화 정책이 더 분명해야 함 | 반영함 | [`docs/plan/05-db-engine-integration.md`](/D:/03Dev/05Jungle/SuYo/Week8/week8-team2-network/docs/plan/05-db-engine-integration.md), [`docs/plan/11-implementation-policies.md`](/D:/03Dev/05Jungle/SuYo/Week8/week8-team2-network/docs/plan/11-implementation-policies.md) |
| graceful shutdown과 queue full 정책이 더 명확해야 함 | 반영함 | [`docs/plan/04-thread-pool-and-concurrency.md`](/D:/03Dev/05Jungle/SuYo/Week8/week8-team2-network/docs/plan/04-thread-pool-and-concurrency.md), [`docs/plan/11-implementation-policies.md`](/D:/03Dev/05Jungle/SuYo/Week8/week8-team2-network/docs/plan/11-implementation-policies.md) |
| 빌드 시스템 책임이 불분명함 | 반영함 | [`docs/plan/11-implementation-policies.md`](/D:/03Dev/05Jungle/SuYo/Week8/week8-team2-network/docs/plan/11-implementation-policies.md) |
| 통합 테스트와 검증 흐름이 약함 | 반영함 | [`docs/plan/07-test-and-quality-plan.md`](/D:/03Dev/05Jungle/SuYo/Week8/week8-team2-network/docs/plan/07-test-and-quality-plan.md), [`docs/plan/11-implementation-policies.md`](/D:/03Dev/05Jungle/SuYo/Week8/week8-team2-network/docs/plan/11-implementation-policies.md) |

## 남는 확인 사항

- 실제 구현 시 `sql_processor`의 세부 오류 코드와 메시지 구조는 다시 한 번 맞춰봐야 한다
- HTTP 파서의 partial read 처리와 응답 길이 계산은 실제 소켓 코드에서 검증해야 한다
- graceful shutdown은 sentinel 방식과 종료 플래그 방식 중 최종 구현 시 하나로 확정해야 한다

## 결론

평가2에서 지적한 핵심은 시간 문제가 아니라 실행 계약의 구체성이었다.
그 부분은 현재 plan에 반영했고, 다음 단계는 이 정책을 기준으로 실제 구현을 시작하는 것이다.
