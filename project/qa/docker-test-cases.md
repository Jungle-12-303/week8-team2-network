# Docker 환경 테스트 케이스

## 목적

도커 환경에서 MVP 서버가 빌드, 실행, 테스트 가능한지 확인한다.

## 공통 전제

- Docker와 Docker Compose가 설치되어 있다.
- 저장소 루트에서 명령을 실행한다.

## 테스트 케이스

### TC-DOCKER-001 이미지 빌드 성공

- 명령:

```bash
docker compose build
```

- 기대 결과:
  - `app` 이미지 빌드 성공
  - `test` 서비스 실행에 필요한 이미지 준비 완료

### TC-DOCKER-002 서버 기동 성공

- 명령:

```bash
docker compose up -d app
```

- 기대 결과:
  - `app` 컨테이너가 실행 상태
  - 호스트 `127.0.0.1:8080` 접근 가능

### TC-DOCKER-003 health endpoint 확인

- 명령:

```bash
curl -i -sS http://127.0.0.1:8080/health
```

- 기대 결과:
  - `200 OK`
  - JSON 응답

### TC-DOCKER-004 smoke test 성공

- 명령:

```bash
docker compose run --rm test
```

- 기대 결과:
  - smoke test 통과
  - `GET /health`, `POST /users`, `GET /users`, `GET /users/{id}` 검증 성공

### TC-DOCKER-005 종료 후 정리

- 명령:

```bash
docker compose down
```

- 기대 결과:
  - 실행 중 컨테이너 정리
  - 다음 실행에 영향이 남지 않음
