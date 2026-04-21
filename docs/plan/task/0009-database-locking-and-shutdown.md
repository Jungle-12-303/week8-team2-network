# DB 락과 종료 처리 정리함

## 목적

공유 DB 자원을 어떻게 보호하고, 서버 종료 시 어떻게 마무리할지 정리한다.

## 범위

- DB mutex 범위 정리함
- lock/unlock 위치 정리함
- graceful shutdown 기준 정리함
- 대기 중인 작업 마무리 기준 정리함

## 완료 기준

- 공유 자원 보호 방식이 분명함
- 종료 시 데이터 처리 경계가 보임
- deadlock 가능성이 줄어듦

## 테스트/검증

- 동시 요청 중 종료를 확인함
- 락이 오래 잡히지 않는지 확인함

## 의존성

- `docs/plan/04-thread-pool-and-concurrency.md`
- `docs/plan/05-db-engine-integration.md`

## 메모

- 동시성 카드 중 가장 조심해서 봐야 하는 항목이다.
