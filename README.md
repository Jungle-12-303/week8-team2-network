# week8-team2-network

TEAM7의 메모리 기반 SQL 처리기와 B+Tree 인덱스를 재사용해 만든 C 기반
미니 DBMS API 서버다.

현재 구현 범위는 플랜의 최소 MVP 기준이다.

- 단일 `users` 테이블
- `GET /health`
- `POST /users`
- `GET /users`
- `GET /users/{id}`
- 고정 크기 thread pool
- worker가 `read -> parse -> execute -> write -> close` 전체 처리
- DB 전체 단위 read-write lock

## Build

```bash
make
```

## Run

기본 포트는 `8080`이다.

```bash
./mini_dbms_server
```

다른 포트로 실행할 수도 있다.

```bash
./mini_dbms_server 8081
```

## API

### Health Check

```bash
curl -i http://127.0.0.1:8080/health
```

### Create User

```bash
curl -i -X POST http://127.0.0.1:8080/users \
  -H 'Content-Type: application/json' \
  -d '{"name":"Alice","age":20}'
```

### List Users

```bash
curl -i http://127.0.0.1:8080/users
```

### Get User By Id

```bash
curl -i http://127.0.0.1:8080/users/1
```

## Test

기본 smoke test는 서버를 띄운 뒤 핵심 endpoint를 순서대로 검증한다.

```bash
make test
```

## Demo

발표 때 가장 안전한 시연 순서는 아래와 같다.

먼저 서버를 실행한다.

```bash
./mini_dbms_server 8080
```

다른 터미널에서 아래 명령을 순서대로 실행한다.

```bash
curl -i http://127.0.0.1:8080/health
curl -i -X POST http://127.0.0.1:8080/users \
  -H 'Content-Type: application/json' \
  -d '{"name":"Alice","age":20}'
curl -i http://127.0.0.1:8080/users
curl -i http://127.0.0.1:8080/users/1
make test
```

이 순서는 아래 항목을 순서대로 보여준다.

- 서버 생존 확인
- 사용자 생성
- 전체 조회
- 단건 조회
- 자동 검증 재현

## Docker

로컬 빌드 환경과 별개로 Docker 기반 실행 경로도 제공한다.

### 이미지 빌드

```bash
make docker-build
```

### 서버 실행

```bash
make docker-up
```

또는:

```bash
docker compose up -d app
```

### Docker smoke test

```bash
make docker-test
```

### 종료

```bash
make docker-down
```

## VS Code Dev Container

`webproxy_lab_docker`처럼 VS Code에서 프로젝트를 열었을 때 Dev Container로
다시 열 수 있도록 `.devcontainer/` 설정도 포함했다.

사용 방법:

1. VS Code로 이 프로젝트 폴더를 연다.
2. Command Palette에서 `Dev Containers: Reopen in Container`를 실행한다.
3. 처음 한 번은 `.devcontainer/Dockerfile` 기준으로 개발 컨테이너를 빌드한다.

컨테이너 안에서는 일반 로컬 실행과 동일하게 아래 명령을 사용하면 된다.

```bash
make
make test
```

## 문서 위치

- [프로젝트 문서 인덱스](/Users/choeyeongbin/week8-team2-network/project/README.md)
- [최소 구현 플랜](/Users/choeyeongbin/week8-team2-network/project/plans/mini-dbms-api-plan.md)
- [API 설계](/Users/choeyeongbin/week8-team2-network/project/architecture/api-spec.md)
- [PM 기능 문서](/Users/choeyeongbin/week8-team2-network/project/features/health-check.md)
- [QA 테스트 케이스](/Users/choeyeongbin/week8-team2-network/project/qa/test-cases.md)
- [Docker 기능 문서](/Users/choeyeongbin/week8-team2-network/project/features/docker-environment.md)
- [Docker 설계 문서](/Users/choeyeongbin/week8-team2-network/project/architecture/docker-runtime.md)
- [Docker 테스트 케이스](/Users/choeyeongbin/week8-team2-network/project/qa/docker-test-cases.md)

## 제한 사항

- 완전한 HTTP/1.1 구현이 아니다.
- 요청당 연결 1회와 `Connection: close`만 지원한다.
- `Content-Length` 기반 body 읽기만 지원한다.
- keep-alive, chunked encoding, query parameter 필터는 지원하지 않는다.
- TEAM7 엔진은 `third_party/team7_engine`에 포함해 서버용으로 연결했다.

## 구조

```text
src/main.c                  서버 시작점
src/server/server.c         소켓 서버, HTTP 처리, 라우팅
src/db/db_adapter.c         TEAM7 SQL 엔진 어댑터 + rwlock
src/concurrency/*.c         thread pool / job queue
third_party/team7_engine/   TEAM7 엔진 코드
tests/smoke_test.sh         기본 endpoint smoke test
Dockerfile                  도커 빌드 설정
docker-compose.yml          도커 실행/테스트 설정
```
