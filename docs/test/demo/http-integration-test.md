# HTTP Integration Test

이 문서는 Docker 기반 HTTP 통합 테스트를 사람이 직접 실행하고 확인하는 방법을 설명합니다.

## 무엇을 테스트하나

`scripts/http_integration_test.sh`는 다음을 한 번에 확인합니다.

- Docker 이미지를 빌드할 수 있는지
- 임시 컨테이너가 정상적으로 뜨는지
- `POST /query`로 `INSERT`가 되는지
- `POST /query`로 `SELECT`가 되는지
- 잘못된 메서드가 `405`를 주는지
- 잘못된 경로가 `404`를 주는지
- 너무 큰 body가 `413`을 주는지

## 수동 실행 방법

프로젝트 루트에서 실행합니다.

```bash
sh scripts/http_integration_test.sh
```

스크립트는 실행 환경에 따라 자동으로 모드를 고릅니다.

- Docker가 있고 데몬에 접근 가능하면 Docker 모드로 실행한다
- Docker를 쓸 수 없으면 `db_server`를 로컬에서 직접 띄워서 실행한다

또는 `make` 타겟으로 실행할 수 있습니다.

```bash
make test-http
```

전체 자동화는 아래로 묶을 수 있습니다.

```bash
make test
```

## 기대 결과

정상적으로 통과하면 마지막에 아래 문구가 출력됩니다.

```text
HTTP integration tests passed.
```

## 실패했을 때 바로 볼 것

- Docker 데몬에 접근 가능한지
- `make db_server`가 성공하는지
- `db_server`가 `18080` 포트에서 뜨는지
- Docker build가 실패했는지
- 서버가 `18080` 포트에서 뜨는지
- `INSERT` 응답에 `ok`, `action`, `inserted_id`, `row_count`가 있는지
- `SELECT` 응답에 `Alice`가 있는지
- `405`, `404`, `413` 응답이 기대값과 맞는지

## 같이 보면 좋은 파일

- [scripts/http_integration_test.sh](../../../scripts/http_integration_test.sh)
- [Makefile](../../../Makefile)
- [docs/plan/generated/http-integration-test-plan.md](../../../docs/plan/generated/http-integration-test-plan.md)
