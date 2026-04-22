# 테스트 데모 안내

이 폴더는 현재 구현되어 있는 테스트를 사람이 직접 따라 해 볼 수 있게 설명하는 곳입니다.

## 현재 있는 테스트

- [단위 테스트](unit-tests.md)
- [HTTP 스모크 테스트와 수동 확인](http-smoke-test.md)
- [HTTP 통합 테스트](http-integration-test.md)

## 원칙

- 이 폴더에는 실제로 실행 가능한 테스트만 적는다
- 아직 구현하지 않은 테스트는 `docs/plan/generated/` 쪽에만 둔다
- 각 문서는 하나의 테스트 종류만 다룬다

## 현재 상태 한 줄 요약

- `sql_processor/unit_test.c`는 이미 구현되어 있다
- `scripts/smoke_test.sh`는 이미 구현되어 있다
- `scripts/http_integration_test.sh`는 이미 구현되어 있다
