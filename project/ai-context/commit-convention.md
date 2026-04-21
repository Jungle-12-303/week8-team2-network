---
name: commit-convention
description: Git 커밋 메시지 작성, 브랜치 전략, PR 워크플로우, merge vs rebase 등 Git 협업 전반을 다루는 문서. 커밋 메시지 제안, 브랜치 전략 결정, PR 설명 작성, 충돌 해결이 필요할 때 우선 참고한다.
---

# Git 워크플로우 & 커밋 컨벤션

## 기본 원칙

- `main`에는 직접 커밋하지 않는다.
- 하나의 커밋은 하나의 의도를 담는다.
- 커밋 메시지는 변경 결과와 이유가 빠르게 드러나야 한다.
- PR은 작고 리뷰 가능해야 한다.
- 공개 브랜치 히스토리는 함부로 재작성하지 않는다.

## 커밋 메시지

### 형식

```text
<type>: <한국어 제목>
```

본문이 필요한 경우:

```text
<type>: <한국어 제목>

<한국어 본문>
```

Breaking Change가 있는 경우:

```text
<type>: <한국어 제목>

<한국어 본문>

BREAKING CHANGE: explain the incompatible change in English
```

### 허용 타입

- `feat`: 사용자 관점의 기능 추가
- `fix`: 버그 수정
- `refactor`: 동작 변화 없이 구조 개선
- `docs`: 문서 변경
- `test`: 테스트 추가 또는 수정
- `chore`: 잡무성 변경, 유지보수
- `style`: 동작에 영향 없는 포맷팅
- `perf`: 성능 개선
- `build`: 빌드 설정, 패키지, 컴파일 구성 변경
- `ci`: CI 설정 변경
- `revert`: 이전 커밋 되돌리기

### 스코프 정책

커밋 제목에는 스코프를 사용하지 않는다.
파일 경로와 한국어 제목으로 영향 범위를 드러낸다.

### 제목 규칙

- 한국어로 작성한다.
- 한 줄로 유지한다.
- 마침표로 끝내지 않는다.
- 행위보다 변경 결과를 서술한다.
- `수정`, `작업`, `변경`, `업데이트` 같은 모호한 표현은 피한다.
- 리뷰어가 제목만 읽어도 결과를 파악할 수 있어야 한다.

좋은 예:

- `fix: 자식 노드 재배치 순서를 바로잡아`
- `build: 정적 라이브러리 출력 경로를 분리해`
- `docs: malloc 실패 처리 규칙을 문서화해`

### 본문 규칙

아래 중 하나라도 해당하면 본문을 추가한다.

- 변경 이유가 제목만으로 충분히 드러나지 않을 때
- 영향 범위가 넓을 때
- 마이그레이션 또는 사용 주의사항이 있을 때
- 리뷰어가 알아야 할 설계 판단이 있을 때

본문은 한국어로 간결하게 작성한다.
필요하면 아래 순서를 따른다.

1. 왜 바꿨는가
2. 무엇이 달라졌는가
3. 리뷰어나 사용자에게 주의할 점이 있는가

### 예시

```text
feat: DOM 렌더러 초기 구조를 추가

fix: 자식 노드 재정렬 시 인덱스 계산 오류를 고쳐

docs: README에 타입스크립트 시작 방법을 정리

build: TypeScript 출력 경로와 타입 선언 생성을 설정

refactor: 가상 노드 생성 흐름을 단순화
```

본문 포함 예시:

```text
fix: 자식 노드 재정렬 시 인덱스 계산 오류를 고쳐

키 비교 뒤 재배치 기준 인덱스를 다시 계산하도록 바꿔
중첩 목록 갱신에서 잘못된 DOM 이동이 발생하지 않게 한다
```

Breaking Change 예시:

```text
feat: 렌더러 초기화 API를 단순화

기본 사용 흐름을 하나로 맞추기 위해 진입 함수를 통합한다

BREAKING CHANGE: replace createRenderer() with createRoot()
```

### 응답 스타일

AI가 커밋 메시지를 제안할 때는 바로 복사 가능한 텍스트 블록으로 반환한다.
필요하면 `이유:` 한 줄만 추가한다.

## 브랜치 전략

### 기본 전략

이 저장소는 기본적으로 GitHub Flow를 따른다.

```text
main
  ├── feature/...
  ├── fix/...
  ├── docs/...
  └── chore/...
```

원칙:

- `main`은 항상 배포 또는 제출 가능한 상태를 유지한다.
- 새 작업은 항상 최신 `main`에서 브랜치를 만든다.
- 작업이 준비되면 PR을 열고 리뷰 뒤 병합한다.
- 오래 살아있는 브랜치는 피하고 가능한 짧게 유지한다.

### 다른 전략을 고려할 때

Trunk-Based Development는 고속 CI/CD와 feature flag가 있을 때만 고려한다.
GitFlow는 릴리즈 브랜치 운영이 꼭 필요한 경우에만 고려한다.

## 브랜치 명명 규칙

```text
feature/user-authentication
feature/JIRA-123-payment-integration

fix/login-redirect-loop
fix/456-null-pointer-exception

docs/memory-layout-guide
chore/update-build-script
hotfix/critical-security-patch
release/1.2.0
```

### 규칙

- 소문자 kebab-case를 기본으로 사용한다.
- 접두사는 작업 성격을 드러내야 한다.
- 이슈 번호가 있으면 앞쪽에 붙여 추적성을 높인다.
- 개인 실험 브랜치는 목적이 드러나도록 짧게 이름 짓는다.

## Merge vs Rebase

### Merge

```bash
git checkout main
git merge feature/user-auth
```

적합한 경우:

- feature 브랜치를 `main`에 병합할 때
- 실제 병합 관계를 보존해야 할 때
- 여러 명이 같은 브랜치에서 작업했을 때
- 이미 공개된 브랜치를 통합할 때

### Rebase

```bash
git checkout feature/user-auth
git fetch origin
git rebase origin/main
```

적합한 경우:

- 내 로컬 브랜치를 최신 `main` 위로 정리할 때
- 아직 공유하지 않은 커밋을 다듬을 때
- 선형 히스토리가 필요할 때

### Rebase 금지 대상

- `main`, `develop` 같은 보호 브랜치
- 이미 다른 사람이 기반으로 사용 중인 공개 브랜치
- 이미 병합된 브랜치

### 충돌 해결 원칙

- 자동 해결 결과를 그대로 믿지 않는다.
- 충돌 후에는 반드시 빌드와 테스트를 다시 돌린다.
- 의미가 바뀌기 쉬운 import, 설정 파일, 조건문, 인덱스 계산부터 다시 본다.

## Pull Request

### PR 제목 형식

```text
<type>: <한국어 설명>
```

예시:

- `feat: 기업 사용자를 위한 SSO 지원 추가`
- `fix: 주문 처리 경쟁 조건 해결`
- `docs: v2 엔드포인트 OpenAPI 명세 추가`

### PR 설명 템플릿

```markdown
## 무엇을 변경했나요?

변경 내용을 간단히 설명.

## 왜 변경했나요?

변경 동기와 맥락 설명.

## 어떻게 구현했나요?

주목할 만한 구현 세부사항.

## 테스트

- [ ] 단위 테스트 추가/수정
- [ ] 통합 테스트 추가/수정
- [ ] 수동 테스트 완료

## 체크리스트

- [ ] 프로젝트 스타일 가이드 준수
- [ ] 자기 리뷰 완료
- [ ] 복잡한 로직에 주석 추가
- [ ] 문서 업데이트
- [ ] 새로운 경고 없음
- [ ] 로컬에서 테스트 통과

Closes #123
```

### PR 작성 원칙

- PR 설명은 코드 diff를 반복하지 말고 맥락을 제공한다.
- 리뷰 포인트가 있으면 별도로 명시한다.
- 스크린샷, 로그, 성능 수치가 중요하면 함께 첨부한다.
- 너무 큰 PR은 먼저 분리 가능성을 검토한다.

## 커밋 단위 가이드

- 리팩터링과 기능 변경은 가능하면 분리한다.
- 포맷팅 전용 변경은 기능 수정과 섞지 않는다.
- rename과 로직 변경이 동시에 크면 두 단계로 나눈다.
- generated file은 소스 변경과 함께 들어가야 하는 경우만 포함한다.

## 자주 쓰는 Git 명령어

```bash
git checkout -b feature/user-auth
git fetch origin
git rebase origin/main

git branch --merged main | grep -v "^\*\|main" | xargs -n 1 git branch -d
git fetch -p

git stash push -m "WIP: 사용자 인증"
git stash pop

git reset --soft HEAD~1
git revert HEAD
git commit --amend -m "수정된 메시지"
```

## 안티패턴

- `main`에 직접 커밋
- `.env`, API 키, 인증 정보 커밋
- 1000줄이 넘는 거대한 PR
- 의미 없는 커밋 메시지
- 공개 브랜치에 무분별한 force push
- 몇 주 이상 유지되는 장기 feature 브랜치
- `dist/`, `node_modules/`, 빌드 산출물 커밋

## 리뷰 전 체크리스트

- 불필요한 디버그 로그가 남아 있지 않은가
- 테스트 또는 재현 절차가 준비되어 있는가
- 문서와 코드가 함께 업데이트되었는가
- 커밋이 읽을 수 있는 단위로 정리되었는가
- 민감 정보가 포함되지 않았는가

## 빠른 참조

| 작업 | 명령어 |
|------|--------|
| 브랜치 생성 | `git checkout -b feature/name` |
| 브랜치 삭제 | `git branch -d branch-name` |
| 브랜치 병합 | `git merge branch-name` |
| 히스토리 보기 | `git log --oneline --graph` |
| 변경사항 확인 | `git diff` |
| 스테이징 | `git add -p` |
| 커밋 | `git commit -m "type: 한국어 제목"` |
| 스태시 | `git stash push -m "설명"` |
| 마지막 커밋 취소 | `git reset --soft HEAD~1` |
