# Test Demo

이 폴더는 현재 구현되어 있는 테스트를 사람이 직접 따라 해 볼 수 있게 설명하는 곳입니다.

## 현재 있는 테스트

- [Unit Tests](unit-tests.md)
- [HTTP Smoke Test and Manual Check](http-smoke-test.md)

## 원칙

- 이 폴더에는 실제로 실행 가능한 테스트만 적는다
- 아직 구현하지 않은 테스트는 `docs/plan/generated/` 쪽에만 둔다
- 각 문서는 하나의 테스트 종류만 다룬다

## 현재 상태 한 줄 요약

- `sql_processor/unit_test.c`는 이미 구현되어 있다
- `scripts/smoke_test.sh`는 이미 구현되어 있다
- 별도의 HTTP 통합 테스트 스크립트는 아직 없다

