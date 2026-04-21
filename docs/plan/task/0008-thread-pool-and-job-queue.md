# 스레드 풀과 작업 큐 정리함

## 목적

동시 요청을 처리하는 기본 구조를 분리해서 정리한다.

## 범위

- worker thread 수 결정함
- job queue 구조 정리함
- enqueue/dequeue 기준 정리함
- queue full 처리 기준 정리함

## 완료 기준

- 요청이 큐로 들어가는 흐름이 보임
- 워커가 처리하는 책임이 분명함
- 큐가 가득 찼을 때의 반응이 정해짐

## 테스트/검증

- 여러 요청을 동시에 넣어 확인함
- 큐가 가득 찼을 때의 응답을 확인함

## 의존성

- `docs/plan/04-thread-pool-and-concurrency.md`
- `docs/plan/06-implementation-order.md`

## 메모

- 요청 처리와 SQL 실행 사이의 중간 레이어다.
