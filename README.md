# week8-team2-network

C 언어로 구현하는 미니 DBMS API 서버 프로젝트입니다. 기존 SQL 처리기와 B+Tree 인덱스를 내부 DB 엔진으로 활용하고, 외부 클라이언트가 API를 통해 SQL 요청을 보낼 수 있도록 서버와 스레드 풀 구조를 설계합니다.

## 문서 목차

### 과제 기준
- [docs/resources/guideLine.md](docs/resources/guideLine.md): 이번 주 과제의 원문 가이드라인입니다. API 서버, 스레드 풀, 기존 SQL 처리기 활용, 테스트, 발표 기준을 확인할 때 가장 먼저 봅니다.

### 구현 계획
- [docs/plan/00-plan-manager.md](docs/plan/00-plan-manager.md): 전체 구현 계획의 최상위 매니저 문서입니다. AI가 어떤 순서로 하위 계획 문서를 읽고 구현해야 하는지 정리합니다.
- [docs/plan/01-requirements-and-scope.md](docs/plan/01-requirements-and-scope.md): 과제 요구사항, 우선순위, 제외 범위를 정리합니다.
- [docs/plan/02-server-architecture.md](docs/plan/02-server-architecture.md): C 기반 API 서버의 전체 아키텍처를 설명합니다.
- [docs/plan/03-api-contract.md](docs/plan/03-api-contract.md): 외부 클라이언트가 호출할 HTTP API 계약을 정의합니다.
- [docs/plan/04-thread-pool-and-concurrency.md](docs/plan/04-thread-pool-and-concurrency.md): 스레드 풀, 작업 큐, 동시성 보호 전략을 정리합니다.
- [docs/plan/05-db-engine-integration.md](docs/plan/05-db-engine-integration.md): 기존 `sql_processor` 엔진을 API 서버에 연결하는 방식을 설명합니다.
- [docs/plan/06-implementation-order.md](docs/plan/06-implementation-order.md): 실제 구현 순서를 단계별로 안내합니다.
- [docs/plan/07-test-and-quality-plan.md](docs/plan/07-test-and-quality-plan.md): 단위 테스트, API 테스트, 동시성 테스트 계획을 정리합니다.
- [docs/plan/08-demo-and-readme-plan.md](docs/plan/08-demo-and-readme-plan.md): 발표와 최종 README 정리 방향을 안내합니다.

### 네트워크 학습 자료
- [docs/study/projects/project-overview.md](docs/study/projects/project-overview.md): 발표자가 프로젝트의 큰그림과 설명 흐름을 빠르게 잡기 위한 개요 문서입니다.
- [docs/study/team/README.md](docs/study/team/README.md): CSAPP 11장, 소켓, Tiny, Proxy 흐름을 학습하기 위한 팀 문서 묶음의 시작점입니다.
- [docs/study/team/01-roadmap.md](docs/study/team/01-roadmap.md): 네트워크 학습 순서와 전체 로드맵입니다.
- [docs/study/team/02-core-concepts.md](docs/study/team/02-core-concepts.md): 주소, 포트, TCP, HTTP, REST API 같은 핵심 개념을 정리합니다.
- [docs/study/team/03-c-for-networking.md](docs/study/team/03-c-for-networking.md): C 문자열, 버퍼, 파일 디스크립터, pthread 등 서버 구현에 필요한 C 개념을 다룹니다.
- [docs/study/team/04-socket-flow.md](docs/study/team/04-socket-flow.md): socket, bind, listen, accept, connect 흐름을 설명합니다.
- [docs/study/team/05-echo-to-tiny.md](docs/study/team/05-echo-to-tiny.md): Echo Server에서 Tiny Web Server로 확장되는 관점을 설명합니다.
- [docs/study/team/06-tiny-webserver-guide.md](docs/study/team/06-tiny-webserver-guide.md): HTTP 요청/응답 서버의 최소 구조를 Tiny 기준으로 정리합니다.
- [docs/study/team/07-self-study-framework.md](docs/study/team/07-self-study-framework.md): 스스로 학습 내용을 점검하기 위한 질문과 프레임워크입니다.
- [docs/study/team/08-proxy-lab-guide.md](docs/study/team/08-proxy-lab-guide.md): Proxy 과제의 요청 중계, 동시성, 캐시 개념을 정리합니다.
- [docs/study/team/09-proxy-implementation-roadmap.md](docs/study/team/09-proxy-implementation-roadmap.md): Proxy 구현 순서와 주의점을 안내합니다.
- [docs/study/team/10-practice-checklist.md](docs/study/team/10-practice-checklist.md): 실습 전후 체크리스트입니다.
- [docs/study/team/11-quiz-and-review.md](docs/study/team/11-quiz-and-review.md): 복습용 퀴즈와 리뷰 질문입니다.
- [docs/study/team/12-glossary.md](docs/study/team/12-glossary.md): 네트워크 용어집입니다.

### 기존 SQL 처리기 자료
- [sql_processor/README.md](sql_processor/README.md): 기존 메모리 기반 SQL 처리기와 B+Tree 인덱스 구현 설명입니다.
- [sql_processor/docs/diagrams](sql_processor/docs/diagrams): SQL 처리기와 B+Tree 동작을 설명하는 Mermaid 다이어그램입니다.
- [sql_processor/docs/images](sql_processor/docs/images): 성능 테스트 결과 이미지 자료입니다.
