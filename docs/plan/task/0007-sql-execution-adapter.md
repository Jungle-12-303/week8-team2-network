# SQL 실행 어댑터 정리함

## 목적

HTTP 요청 본문과 기존 SQL 처리기 사이의 연결부를 정리한다.

## 범위

- `sql_execute()` 호출 방식 확인함
- 입력 문자열 전달 방식 정리함
- 결과 구조 변환 기준 정리함
- INSERT/SELECT 분기 정리함

## 완료 기준

- SQL 실행 경로가 명확함
- 결과를 HTTP 응답으로 바꾸는 기준이 보임
- 기존 SQL 처리기와의 경계가 분명함

## 테스트/검증

- `INSERT` 실행을 확인함
- `SELECT` 실행을 확인함
- SQL 에러가 HTTP 에러로 바뀌는지 확인함

## 의존성

- `docs/plan/05-db-engine-integration.md`
- `docs/plan/06-implementation-order.md`

## 메모

- 기존 엔진을 크게 건드리지 않는 방식으로 간다.
