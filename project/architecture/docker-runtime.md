# Docker Runtime Design

## 목적

MVP 서버를 로컬 개발 환경과 분리된 일관된 컨테이너 환경에서 실행하고 검증할 수 있도록 최소한의 도커 실행 구조를 정의한다.

## 설계 원칙

- 기존 로컬 빌드/실행 방식을 유지한다.
- 도커 환경은 추가 실행 경로로 제공한다.
- 컨테이너 안에서도 실제 실행 명령은 `make`와 `./mini_dbms_server`를 기준으로 유지한다.
- 멀티 스테이지나 런타임 최소화보다 구현 단순성을 우선한다.

## 구성

### 1. `app` 서비스

- 역할: 서버 빌드 및 실행
- 포트: 컨테이너 `8080` -> 호스트 `8080`
- 시작 명령: `make && ./mini_dbms_server 8080`

### 2. `test` 서비스

- 역할: smoke test 실행
- 의존: `app`
- 실행 명령: `bash tests/smoke_test.sh`

## 데이터 흐름

```text
host curl
-> localhost:8080
-> docker published port
-> app container
-> mini_dbms_server
-> TEAM7 engine
-> JSON response
```

## 구현 메모

- 서버는 메모리 기반이므로 볼륨 마운트가 필수는 아니다.
- 다만 개발 편의를 위해 소스 코드를 컨테이너에서 빌드 가능하게 구성한다.
- smoke test는 컨테이너 네트워크에서 `app:8080`을 대상으로 호출하도록 분리하는 것이 안정적이다.

## 선택 이유

- `Dockerfile` 하나와 `docker-compose.yml` 하나면 팀원이 동일 절차로 재현할 수 있다.
- 실행 환경 차이로 인한 컴파일 문제를 줄일 수 있다.
- QA와 발표 시 “같은 명령으로 누구나 실행 가능하다”는 근거를 제공한다.
