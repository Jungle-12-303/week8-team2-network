# 버킷 락 동시성 테스트 안내

이 문서는 현재 구현된 해시 버킷 단위 `pthread_rwlock_t`가 실제로 잘 동작하는지 확인하는 방법을 설명한다.

> 이 문서는 해시 버킷 단위 동시성 테스트를 설명한다.

## 무엇을 확인하나

- 여러 `SELECT`가 동시에 들어와도 읽기 요청이 서로 막히지 않는지 확인한다.
- 여러 `INSERT`가 동시에 들어와도 데이터가 중복되거나 누락되지 않는지 확인한다.
- `SELECT`와 `INSERT`가 섞여도 결과 순서와 최종 row 수가 유지되는지 확인한다.
- 종료 처리 중에도 서버가 멈추지 않는지 확인한다.

## 실행 스크립트

- `sh scripts/tests/concurrency/bucket-lock-stress-test.sh`
- 호환용 루트 래퍼: `sh scripts/bucket_lock_stress_test.sh`

## 현재 구현 기준

- `id % TABLE_BUCKET_COUNT`로 버킷을 고른다.
- 각 버킷은 자기만의 `pthread_rwlock_t`를 가진다.
- `SELECT`와 `INSERT`는 서로 다른 버킷이면 동시에 진행될 수 있다.
- 이 테스트는 전역 `db_lock`이 아니라 **버킷 단위 락**이 잘 풀리는지를 본다.

## 실행 흐름

1. 서버가 응답할 때까지 기다린다.
2. seed row 1개를 먼저 넣는다.
3. `SELECT`와 `INSERT`를 32쌍 동시에 보낸다.
4. 모든 응답에 `"ok":true`가 있는지 확인한다.
5. 마지막 `SELECT * FROM users;`에서 `row_count == 33`인지 확인한다.

## 기대 결과

다음 조건이 모두 맞으면 성공이다.

- 중간 요청이 실패하지 않는다.
- 최종 `row_count`가 33이다.
- 버킷 충돌이 있어도 결과가 누락되지 않는다.
- 마지막 출력이 `bucket lock stress test passed.`이다.

## 관련 코드

- [`sql_processor/table.h`](../../../sql_processor/table.h)
- [`sql_processor/table.c`](../../../sql_processor/table.c)
- [`server/api.c`](../../../server/api.c)
- [`server/server.c`](../../../server/server.c)
- [`scripts/tests/concurrency/bucket-lock-stress-test.sh`](../../../scripts/tests/concurrency/bucket-lock-stress-test.sh)
