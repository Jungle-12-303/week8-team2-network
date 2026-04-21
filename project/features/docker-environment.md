# Docker Environment

## 목적

도커 환경 기능은 팀원이 로컬 개발 도구 차이와 무관하게 같은 방식으로 서버를 빌드하고 실행할 수 있게 하는 것을 목표로 한다.

이 기능은 서버 구현 자체를 바꾸는 것이 아니라, 플랜에 정의된 MVP 서버를
일관된 환경에서 실행하고 검증할 수 있게 만드는 실행 기반을 제공한다.

## 사용자 관점 기대 동작

- 팀원은 로컬에 C 빌드 환경을 직접 맞추지 않아도 도커로 서버를 실행할 수 있다.
- `docker compose up`으로 서버를 띄우고, 호스트에서 `curl`로 API를 검증할 수 있다.
- `docker compose run` 또는 동등한 명령으로 smoke test를 실행할 수 있다.
- 도커 환경에서도 README와 QA 문서에 적힌 절차가 그대로 재현 가능해야 한다.

## 요청 입력

도커 환경 기능은 HTTP endpoint가 아니라 실행 환경 기능이다.

입력 형태:

- `docker compose up --build`
- `docker compose down`
- `docker compose run --rm test`

## 응답 출력

### 성공 기준

- 서버 컨테이너가 정상 기동한다.
- 호스트의 `127.0.0.1:8080`에서 API 접근이 가능하다.
- 테스트 컨테이너 또는 동등한 도커 실행 경로에서 smoke test가 통과한다.

### 실패 기준

- 이미지 빌드 실패
- 서버 프로세스 시작 실패
- 포트 매핑 실패
- smoke test 실패

## Acceptance Criteria

- 저장소에 `Dockerfile`이 존재한다.
- 저장소에 `docker-compose.yml`이 존재한다.
- 서버 컨테이너는 프로젝트 바이너리를 빌드하고 실행할 수 있다.
- 기본 포트 `8080`이 호스트에 노출된다.
- 도커 환경에서 smoke test를 실행할 수 있다.
- README에 도커 빌드, 실행, 종료, 테스트 방법이 문서화되어 있다.
- 기존 로컬 실행 방식(`make`, `./mini_dbms_server`)은 유지된다.

## 구현해야 할 항목

- 서버 실행용 `Dockerfile`
- 실행과 테스트를 위한 `docker-compose.yml`
- 불필요한 산출물을 제외하는 `.dockerignore`
- 도커 관련 `Makefile` target 또는 README 실행 절차
- QA 문서에 도커 검증 절차 추가

## 제외 범위

- 프로덕션 배포용 오케스트레이션
- 멀티 컨테이너 DB 분리 구조
- 이미지 최적화에 대한 고급 튜닝
- 쿠버네티스 배포
- 컨테이너 헬스체크 자동 재시작 정책
