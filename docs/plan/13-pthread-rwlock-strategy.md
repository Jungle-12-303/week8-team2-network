# 13. pthread_rwlock_t 전환 전략

## 목적

현재 서버는 `db_mutex`로 모든 SQL 요청을 한 줄로 세워 처리하고 있다.
이 구조는 구현은 단순하지만, `SELECT`처럼 읽기만 하는 요청까지 서로 막아버린다.

이 문서는 데이터베이스 보호 수단을 `pthread_mutex_t`에서 `pthread_rwlock_t`로 바꾸는 기준을 정의한다.

## 전환 이유

- `SELECT`는 서로 독립적인 읽기 요청이라 동시에 처리해도 된다.
- `INSERT`는 테이블을 수정하므로 배타적으로 처리해야 한다.
- 현재 `sql_processor`는 읽기와 쓰기가 명확히 나뉘므로 `rwlock`이 잘 맞는다.
- 병렬성이 CPU 개수보다 락 경쟁에 더 크게 좌우되는 구조에서, 읽기 요청을 공유 락으로 열어두는 편이 훨씬 유리하다.

## 정책

- `SELECT` 계열 요청은 `pthread_rwlock_rdlock()`을 사용한다.
- `INSERT` 계열 요청은 `pthread_rwlock_wrlock()`을 사용한다.
- `EXIT` / `QUIT` / 알 수 없는 문장은 DB 락을 잡지 않는다.
- JSON 응답 생성은 락 밖에서 수행한다.
- SQL 실행 중 테이블 접근이 필요한 구간만 락으로 감싼다.

## 구현 포인트

1. `server`의 DB 보호 필드를 `pthread_rwlock_t`로 교체한다.
2. SQL 문자열의 첫 토큰을 보고 lock mode를 미리 결정한다.
3. `api_handle_query()`는 lock mode에 따라 read lock 또는 write lock을 건다.
4. SQL 실행이 끝나면 바로 unlock 하고 JSON 직렬화는 밖에서 처리한다.
5. `server_destroy()`와 초기화 경로도 `pthread_rwlock_init()` / `pthread_rwlock_destroy()`로 정리한다.

## 기대 효과

- 여러 `SELECT` 요청이 동시에 진행될 수 있다.
- `INSERT` 중에도 다른 `SELECT`가 불필요하게 막히지 않는다.
- 락 점유 시간이 짧아져 워커 수가 적어도 처리량이 더 안정적이다.

## 남는 제약

- `INSERT`는 여전히 배타 락이 필요하다.
- 테이블 구조 자체가 읽기 전용 캐시가 아니므로, `SELECT`와 `INSERT`가 같은 시점에 충돌하지 않도록 반드시 `rwlock`을 거쳐야 한다.
- 향후 `UPDATE` / `DELETE`를 넣게 되면 쓰기 락 범주로 함께 묶어야 한다.

