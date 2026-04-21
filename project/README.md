# Project Docs

`project` 디렉터리는 이번 과제 수행에 필요한 문서를 목적별로 정리해 두는 공간이다.

## 디렉터리 구조

- `ai-context/`: AI가 코딩 전 반드시 읽어야 하는 규약과 작업 기준
- `architecture/`: API 명세, 요청 흐름, 모듈 경계
- `features/`: PM 기능 정의 문서
- `personas/`: 역할별 책임, 산출물, 완료 기준
- `plans/`: 구현 계획, 아키텍처 초안, 단계별 일정
- `qa/`: 테스트 케이스, 결과 템플릿, 검증 기록

## 현재 문서

- [AI Context Docs](/Users/choeyeongbin/week8-team2-network/project/ai-context/README.md)
- [Architecture Docs](/Users/choeyeongbin/week8-team2-network/project/architecture/README.md)
- [Feature Docs](/Users/choeyeongbin/week8-team2-network/project/features/health-check.md)
- [Personas](/Users/choeyeongbin/week8-team2-network/project/personas/README.md)
- [미니 DBMS API 서버 구현 플랜](/Users/choeyeongbin/week8-team2-network/project/plans/mini-dbms-api-plan.md)
- [QA Test Cases](/Users/choeyeongbin/week8-team2-network/project/qa/test-cases.md)

## 운영 원칙

- 규칙성 문서는 `ai-context/`에 둔다.
- 설계 산출물은 `architecture/`에 둔다.
- PM 기능 문서는 `features/`에 둔다.
- 역할과 협업 흐름 정의는 `personas/`에 둔다.
- 프로젝트별 실행 계획과 설계안은 `plans/`에 둔다.
- QA 산출물은 `qa/`에 둔다.
- 실제 개발을 시작하기 전에 `project/ai-context` 문서를 먼저 읽고 그 기준을 반영한다.
- 구현 후 커밋까지를 개발 작업의 일부로 보고, 커밋 메시지와 단위도 문서 규칙에 맞춘다.
- 커밋은 기능, 리팩터링, 문서, 테스트를 적절히 분리해 리뷰 가능한 단위로 나눈다.
- 발표 자료의 근거가 되는 내용은 README 또는 `plans/` 문서에서 바로 찾을 수 있어야 한다.
