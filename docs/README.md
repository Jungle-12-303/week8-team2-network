# docs 안내

이 폴더에는 프로젝트를 이해하고, 계획하고, 학습하고, 기록하기 위한 문서가 모여 있다.

## 먼저 볼 문서

1. [resources/guideLine.md](resources/guideLine.md) - 과제 가이드라인
2. [plan/00-plan-manager.md](plan/00-plan-manager.md) - 전체 계획의 출발점
3. [study/projects/project-overview.md](study/projects/project-overview.md) - 프로젝트 개요
4. [study/team/README.md](study/team/README.md) - 팀 학습 문서 진입점
5. [Review/README.md](Review/README.md) - 비교, 리뷰, 해결기록 문서 진입점

## 현재 구조

```text
docs/
  README.md
  resources/
  plan/
  study/
  Review/
```

## resources

과제 기준, 참고 문서, 외부 안내를 모아둔 폴더다.

- [resources/guideLine.md](resources/guideLine.md): 이번 과제의 요구사항과 평가 기준

## plan

구현 전에 어떤 순서로 만들지 정리한 계획 문서 폴더다.

- [plan/00-plan-manager.md](plan/00-plan-manager.md): 전체 계획의 시작점
- [plan/01-requirements-and-scope.md](plan/01-requirements-and-scope.md): 요구사항과 범위
- [plan/02-server-architecture.md](plan/02-server-architecture.md): 서버 구조
- [plan/03-api-contract.md](plan/03-api-contract.md): HTTP API 계약
- [plan/04-thread-pool-and-concurrency.md](plan/04-thread-pool-and-concurrency.md): 스레드 풀과 동시성
- [plan/05-db-engine-integration.md](plan/05-db-engine-integration.md): DB 엔진 연동
- [plan/06-implementation-order.md](plan/06-implementation-order.md): 구현 순서
- [plan/07-test-and-quality-plan.md](plan/07-test-and-quality-plan.md): 테스트와 품질
- [plan/08-demo-and-readme-plan.md](plan/08-demo-and-readme-plan.md): 발표와 README 정리

## study

구현을 이해하기 위한 학습 문서 폴더다.

- [study/projects/project-overview.md](study/projects/project-overview.md): 프로젝트 전체 개요
- [study/team/README.md](study/team/README.md): 팀 학습 문서 시작점

## Review

비교 리포트, 리뷰 반영 기록, 해결기록 가이드를 모아둔 폴더다.

- [Review/README.md](Review/README.md): Review 문서 인덱스
- [Review/혜연vsexp1.md](Review/혜연vsexp1.md): 브랜치 비교 리포트
- [Review/초기리뷰/](Review/초기리뷰/): 초기 리뷰 반영 기록
- [Review/해결기록/README.md](Review/해결기록/README.md): 해결기록 사용 가이드

## 문서 작성 원칙

- 새 문서를 만들면 반드시 상위 인덱스 링크도 함께 갱신한다.
- 계획 문서는 `plan/`, 학습 문서는 `study/`, 리뷰 문서는 `Review/`에 둔다.
- 구현 기록은 문제를 먼저 쓰고, 학습과 해결 과정을 이어서 적는다.
