# docs 안내 문서

이 문서는 `docs` 폴더 아래에 있는 문서와 폴더의 역할을 설명하는 가이드입니다. 새로 들어온 팀원이나 AI가 프로젝트 문서를 읽을 때는 이 문서를 먼저 보고, 목적에 맞는 하위 문서로 이동하면 됩니다.

## 먼저 읽을 문서

1. [resources/guideLine.md](resources/guideLine.md): 과제 원문 가이드라인입니다. 왜 이 프로젝트를 하는지, 필수 요구사항이 무엇인지 확인합니다.
2. [plan/00-plan-manager.md](plan/00-plan-manager.md): 구현 계획의 최상위 매니저 문서입니다. AI가 어떤 순서로 하위 플랜을 읽고 구현해야 하는지 안내합니다.
3. [study/projects/project-overview.md](study/projects/project-overview.md): 발표자가 프로젝트의 큰그림을 빠르게 이해하기 위한 개요 문서입니다.
4. [study/team/README.md](study/team/README.md): 네트워크와 서버 구현을 이해하기 위한 팀 스터디 문서의 시작점입니다.

## 현재 폴더 구조

```text
docs/
  README.md
  resources/
  plan/
  study/
  project/
```

## resources

[resources](resources)는 과제의 기준이 되는 원문 자료를 보관하는 폴더입니다.

- [resources/guideLine.md](resources/guideLine.md): 미니 DBMS API 서버 과제의 목표, 요구사항, 중점 포인트, 품질 기준, 발표 기준을 담고 있습니다.

사용 기준:

- 구현 방향이 흔들릴 때 가장 먼저 확인합니다.
- 기능 우선순위를 정할 때 기준으로 삼습니다.
- 발표와 README에 반드시 들어가야 하는 내용을 확인합니다.

## plan

[plan](plan)은 실제 구현을 위한 작업 계획 문서를 보관하는 폴더입니다. 바이브코딩을 할 때 AI가 가장 자주 참조할 문서 묶음입니다.

- [plan/00-plan-manager.md](plan/00-plan-manager.md): 전체 플랜의 진입점입니다. 가이드라인 항목별 매핑과 AI 참조 순서를 담고 있습니다.
- [plan/01-requirements-and-scope.md](plan/01-requirements-and-scope.md): 요구사항, MVP 범위, 제외 범위를 정리합니다.
- [plan/02-server-architecture.md](plan/02-server-architecture.md): API 서버의 큰 구조와 계층 책임을 설명합니다.
- [plan/03-api-contract.md](plan/03-api-contract.md): 외부 클라이언트가 호출할 HTTP API 계약을 정의합니다.
- [plan/04-thread-pool-and-concurrency.md](plan/04-thread-pool-and-concurrency.md): 스레드 풀, 작업 큐, DB mutex 정책을 설명합니다.
- [plan/05-db-engine-integration.md](plan/05-db-engine-integration.md): 기존 SQL 처리기와 B+Tree 엔진을 서버에 연결하는 방식을 설명합니다.
- [plan/06-implementation-order.md](plan/06-implementation-order.md): 실제 구현 순서를 단계별로 안내합니다.
- [plan/07-test-and-quality-plan.md](plan/07-test-and-quality-plan.md): 단위 테스트, API 테스트, 동시성 테스트 계획을 정리합니다.
- [plan/08-demo-and-readme-plan.md](plan/08-demo-and-readme-plan.md): 발표와 최종 README 정리 방향을 안내합니다.

사용 기준:

- 구현을 시작할 때는 반드시 `00-plan-manager.md`부터 봅니다.
- 코드 작성 시 모든 plan 문서의 “코드 주석 원칙”을 따릅니다.
- 새 기능을 추가하기 전에 해당 기능이 어느 플랜 문서에 속하는지 확인합니다.

## study

[study](study)는 구현에 필요한 배경지식과 발표자가 큰그림을 이해하기 위한 학습 문서를 보관하는 폴더입니다.

### study/projects

[study/projects](study/projects)는 이번 프로젝트 자체를 이해하기 위한 개요 문서를 담습니다.

- [study/projects/project-overview.md](study/projects/project-overview.md): 미니 DBMS API 서버의 목적, 전체 흐름, 구성요소, 발표 포인트를 정리합니다.

사용 기준:

- 발표자가 프로젝트를 처음 설명하기 전에 읽습니다.
- 팀원이 “이 프로젝트가 무엇을 만드는지” 빠르게 파악할 때 사용합니다.
- README 발표 흐름을 잡을 때 참고합니다.

### study/team

[study/team](study/team)은 네트워크 서버 구현에 필요한 팀 스터디 문서 묶음입니다.

- [study/team/README.md](study/team/README.md): 팀 스터디 문서의 시작점입니다.
- [study/team/01-roadmap.md](study/team/01-roadmap.md): 학습 순서와 전체 로드맵입니다.
- [study/team/02-core-concepts.md](study/team/02-core-concepts.md): TCP, HTTP, REST API 등 핵심 개념입니다.
- [study/team/03-c-for-networking.md](study/team/03-c-for-networking.md): C 네트워크 프로그래밍에 필요한 개념입니다.
- [study/team/04-socket-flow.md](study/team/04-socket-flow.md): socket, bind, listen, accept, connect 흐름입니다.
- [study/team/05-echo-to-tiny.md](study/team/05-echo-to-tiny.md): echo server에서 tiny web server로 확장되는 관점입니다.
- [study/team/06-tiny-webserver-guide.md](study/team/06-tiny-webserver-guide.md): HTTP 서버 최소 구조를 Tiny 기준으로 설명합니다.
- [study/team/07-self-study-framework.md](study/team/07-self-study-framework.md): 자기 점검 질문과 학습 프레임워크입니다.
- [study/team/08-proxy-lab-guide.md](study/team/08-proxy-lab-guide.md): proxy 과제의 동시성, 중계, 캐시 개념입니다.
- [study/team/09-proxy-implementation-roadmap.md](study/team/09-proxy-implementation-roadmap.md): proxy 구현 로드맵입니다.
- [study/team/10-practice-checklist.md](study/team/10-practice-checklist.md): 실습 체크리스트입니다.
- [study/team/11-quiz-and-review.md](study/team/11-quiz-and-review.md): 복습 퀴즈와 리뷰 질문입니다.
- [study/team/12-glossary.md](study/team/12-glossary.md): 네트워크 용어집입니다.

### study/CYB, study/IHY, study/IJC, study/JHS

각 팀원의 개인 정리 문서가 들어 있는 폴더입니다.

- [study/CYB/11.1-11.4-study.md](study/CYB/11.1-11.4-study.md)
- [study/IHY/11.1-11.4-study.md](study/IHY/11.1-11.4-study.md)
- [study/IJC/11.1-11.4-study.md](study/IJC/11.1-11.4-study.md)
- [study/JHS/11.1-11.4-study.md](study/JHS/11.1-11.4-study.md)

사용 기준:

- 팀원별 이해 방식과 정리 내용을 비교할 때 사용합니다.
- 발표나 리뷰 때 특정 개념을 다른 표현으로 설명하고 싶을 때 참고합니다.

## project

[project](project)는 프로젝트 진행 중 생기는 산출물을 모으기 위한 폴더입니다. 현재는 별도 문서가 없지만, 앞으로 구현 기록이나 프로젝트 운영 문서를 넣기 좋은 위치입니다.

추천 용도:

- 구현 중 결정한 내용의 기록.
- 발표 준비 자료.
- 데모 스크립트.
- 회의 기록.
- 팀별 역할 분담.

## 읽는 목적별 추천 경로

### AI에게 구현을 맡길 때

1. [resources/guideLine.md](resources/guideLine.md)
2. [plan/00-plan-manager.md](plan/00-plan-manager.md)
3. [plan/06-implementation-order.md](plan/06-implementation-order.md)
4. [plan/07-test-and-quality-plan.md](plan/07-test-and-quality-plan.md)

### 발표자가 큰그림을 잡을 때

1. [study/projects/project-overview.md](study/projects/project-overview.md)
2. [plan/00-plan-manager.md](plan/00-plan-manager.md)
3. [plan/08-demo-and-readme-plan.md](plan/08-demo-and-readme-plan.md)

### 서버 구현 개념을 복습할 때

1. [study/team/02-core-concepts.md](study/team/02-core-concepts.md)
2. [study/team/03-c-for-networking.md](study/team/03-c-for-networking.md)
3. [study/team/04-socket-flow.md](study/team/04-socket-flow.md)
4. [study/team/06-tiny-webserver-guide.md](study/team/06-tiny-webserver-guide.md)

## 앞으로 있으면 좋은 폴더

현재 구조만으로도 계획과 학습 문서는 충분히 나뉘어 있습니다. 다만 구현이 진행되면 아래 성격의 폴더가 추가되면 좋습니다.

### architecture

서버 구조, 요청 흐름, 스레드 풀, DB 엔진 연결을 다이어그램 중심으로 설명하는 폴더입니다.

추천 문서:

- `architecture/server-flow.md`
- `architecture/thread-pool.md`
- `architecture/db-engine-boundary.md`

### api

API 명세와 curl 예시를 구현 결과에 맞춰 관리하는 폴더입니다.

추천 문서:

- `api/query-api.md`
- `api/error-response.md`
- `api/curl-examples.md`

### tests

테스트 방법, 테스트 결과, 동시성 검증 기록을 모으는 폴더입니다.

추천 문서:

- `tests/unit-test-results.md`
- `tests/api-test-scenarios.md`
- `tests/concurrency-test-results.md`

### decisions

프로젝트 중 내린 중요한 설계 결정을 기록하는 폴더입니다. 왜 HTTP를 선택했는지, 왜 전역 DB mutex를 썼는지 같은 결정을 남기면 발표와 회고에 도움이 됩니다.

추천 문서:

- `decisions/001-http-api.md`
- `decisions/002-thread-pool.md`
- `decisions/003-db-mutex.md`

### demo

발표와 시연에 필요한 스크립트, 순서, 예상 출력 예시를 보관하는 폴더입니다.

추천 문서:

- `demo/presentation-flow.md`
- `demo/curl-demo-script.md`
- `demo/expected-output.md`

### troubleshooting

구현 중 자주 만나는 문제와 해결 방법을 정리하는 폴더입니다.

추천 문서:

- `troubleshooting/socket-errors.md`
- `troubleshooting/thread-deadlock.md`
- `troubleshooting/http-parsing.md`

## 문서 작성 규칙

- 새 문서를 만들 때는 이 README에서 링크할 수 있도록 경로와 목적을 함께 추가합니다.
- 구현 계획 문서는 [plan](plan)에 둡니다.
- 학습 자료는 [study](study)에 둡니다.
- 과제 원문이나 외부 참고 기준은 [resources](resources)에 둡니다.
- 프로젝트 산출물, 발표 자료, 운영 기록은 [project](project)에 둡니다.
- 코드 구현과 직접 연결되는 문서에는 “한국어 주석을 친절히 작성한다”는 원칙을 유지합니다.
