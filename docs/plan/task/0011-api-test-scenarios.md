# API 테스트 시나리오 정리함

## 목적

HTTP API 기준의 시나리오를 구체화한다.

## 범위

- 정상 `POST /query` 확인함
- `SELECT` 응답 확인함
- `INSERT` 응답 확인함
- malformed request 확인함
- 404/405/400/413 응답 확인함

## 완료 기준

- 성공 경로와 실패 경로가 분리됨
- curl 기준으로 재현 가능함
- 문서와 테스트가 연결됨

## 테스트/검증

- curl 스크립트로 직접 확인함
- 응답 코드와 body를 확인함

## 의존성

- `docs/plan/03-api-contract.md`
- `docs/plan/07-test-and-quality-plan.md`

## 메모

- 발표 데모와도 잘 맞는다.
