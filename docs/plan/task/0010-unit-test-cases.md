# 단위 테스트 케이스 정리함

## 목적

기존 SQL 처리기와 핵심 로직의 단위 테스트 항목을 세분화한다.

## 범위

- B+Tree insert/search 확인함
- duplicate key 확인함
- leaf split 확인함
- internal split 확인함
- range query 확인함

## 완료 기준

- 핵심 자료구조 동작이 보임
- 실패 케이스가 적어도 하나씩 있음
- 테스트가 기능 단위로 나뉨

## 테스트/검증

- `sql_processor` 단위 테스트 실행함
- 실패 케이스를 추가할 후보를 적음

## 의존성

- `docs/plan/07-test-and-quality-plan.md`

## 메모

- 기존 테스트 계획을 실행 카드로 내리는 용도다.
