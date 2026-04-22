# 테스트 데모 안내

이 폴더에는 서버를 직접 돌리면서 확인하는 테스트 문서를 모아둔다.

## 현재 문서

- [단위 테스트](unit-tests.md)
- [HTTP 스모크 테스트](http-smoke-test.md)
- [HTTP 통합 테스트](http-integration-test.md)
- [수동 쿼리 확인](manual-query.md)
- [rwlock 수동 테스트](rwlock-test.md)

## 사용 원칙

- 실제로 실행 가능한 테스트만 둔다.
- 아직 구현되지 않은 테스트는 `docs/plan/generated/`로 분리한다.
- 실패 경로와 정상 경로를 둘 다 확인한다.

## 현재 상태 요약

- `sql_processor/unit_test.c`는 구현되어 있다.
- `scripts/smoke_test.sh`는 구현되어 있다.
- `scripts/http_integration_test.sh`는 구현되어 있다.
- `scripts/manual_query.sh`는 구현되어 있다.
- `scripts/rwlock_stress_test.sh`는 구현되어 있다.
