# 00. Plan Manager

## 코드 주석 원칙

이 계획에 따라 코드를 작성할 때는 핵심 흐름, 동시성 제어, 메모리 소유권, 오류 처리 지점에 한국어 주석을 친절하게 남깁니다. 단순히 코드가 무엇을 하는지 반복하기보다, 왜 해당 잠금이 필요한지, 어떤 버퍼가 어떤 요청 구간을 담는지, `SQLResult`와 `Record`의 소유권이 어디에 있는지처럼 구현자가 이해해야 하는 의도를 설명합니다.

이 문서는 미니 DBMS API 서버 구현의 최상위 매니저 문서입니다. AI 또는 팀원이 바이브코딩을 시작할 때는 이 문서를 먼저 읽고, 아래 순서대로 하위 문서를 참조합니다.

## 목표

[과제 가이드라인](../resources/guideLine.md)의 요구사항을 구현 가능한 작업 순서로 바꿉니다.

- 기존 [sql_processor](../../sql_processor) SQL 처리기와 B+Tree 인덱스를 내부 DB 엔진으로 사용합니다.
- C 언어로 외부 클라이언트가 접근할 수 있는 API 서버를 만듭니다.
- 스레드 풀을 구성해 여러 SQL 요청을 병렬로 처리합니다.
- 테스트와 발표 자료는 최종 [README.md](../../README.md)를 기준으로 설명할 수 있게 정리합니다.

## 가이드라인 매핑

| 가이드라인 항목 | 구현 계획 문서 | 구현 관점 |
| --- | --- | --- |
| 미니 DBMS - API 서버 구현 | [02-server-architecture.md](02-server-architecture.md), [03-api-contract.md](03-api-contract.md) | 서버 구조와 외부 API 정의 |
| 외부 클라이언트에서 DBMS 기능 사용 | [03-api-contract.md](03-api-contract.md) | HTTP 요청/응답 계약 |
| 스레드 풀로 SQL 요청 병렬 처리 | [04-thread-pool-and-concurrency.md](04-thread-pool-and-concurrency.md) | worker, job queue, mutex, condition variable |
| 기존 SQL 처리기와 B+Tree 활용 | [05-db-engine-integration.md](05-db-engine-integration.md) | `sql_execute()` 중심 통합 |
| C 언어 구현 | [02-server-architecture.md](02-server-architecture.md), [06-implementation-order.md](06-implementation-order.md) | POSIX socket, pthread 기반 구현 |
| 구현 우선, 핵심 로직 설명 가능 | [01-requirements-and-scope.md](01-requirements-and-scope.md), [08-demo-and-readme-plan.md](08-demo-and-readme-plan.md) | MVP 우선순위와 발표 설명 포인트 |
| 단위/API/엣지 케이스 테스트 | [07-test-and-quality-plan.md](07-test-and-quality-plan.md) | 기존 테스트 유지와 서버 테스트 추가 |
| README 기반 발표 | [08-demo-and-readme-plan.md](08-demo-and-readme-plan.md) | 실행 방법, 데모, 테스트 결과 정리 |

## AI 참조 순서

1. [01-requirements-and-scope.md](01-requirements-and-scope.md)를 읽고 이번 구현의 성공 기준과 제외 범위를 고정합니다.
2. [02-server-architecture.md](02-server-architecture.md)를 읽고 서버의 큰 흐름을 잡습니다.
3. [03-api-contract.md](03-api-contract.md)를 읽고 HTTP 요청/응답 형태를 먼저 고정합니다.
4. [04-thread-pool-and-concurrency.md](04-thread-pool-and-concurrency.md)를 읽고 병렬 처리와 잠금 정책을 결정합니다.
5. [05-db-engine-integration.md](05-db-engine-integration.md)를 읽고 기존 SQL 엔진을 어떻게 호출할지 확인합니다.
6. [06-implementation-order.md](06-implementation-order.md)를 따라 작은 단계로 구현합니다.
7. [07-test-and-quality-plan.md](07-test-and-quality-plan.md)를 기준으로 테스트를 추가하고 검증합니다.
8. [08-demo-and-readme-plan.md](08-demo-and-readme-plan.md)를 보고 최종 README와 발표 데모를 정리합니다.

## 구현 기본값

- API는 HTTP 기반으로 시작합니다.
- 첫 버전은 단일 엔드포인트 `POST /query`로 SQL 문자열을 받습니다.
- 서버 프로세스 안에는 하나의 공유 `Table` 인스턴스를 둡니다.
- 공유 DB 엔진 접근은 전역 DB mutex로 보호합니다.
- 동시성 차별점은 thread-per-request가 아니라 고정 크기 thread pool과 bounded job queue로 보여줍니다.

## 완료 기준

- 외부 클라이언트가 API로 `INSERT`, `SELECT` SQL을 실행할 수 있습니다.
- 여러 클라이언트 요청이 스레드 풀을 통해 처리됩니다.
- 기존 SQL 처리기 단위 테스트가 계속 통과합니다.
- API 기능 테스트와 동시 요청 테스트를 README에 설명할 수 있습니다.
