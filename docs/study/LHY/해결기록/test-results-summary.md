# 테스트 결과 요약

## 1. 요약

이번 수정에서는 두 가지를 검증했다.

1. HTTP 입력이 너무 느릴 때 worker가 오래 묶이지 않도록 `SO_RCVTIMEO` / `SO_SNDTIMEO`를 적용했다.
2. 테이블 전체 락을 버킷 단위 락으로 분리한 뒤, 동시성 테스트가 여전히 안정적으로 통과하는지 확인했다.

---

## 2. 구현 반영 내용

### 2-1. 소켓 타임아웃

- `server/server.c`에서 클라이언트 소켓에 `SO_RCVTIMEO`와 `SO_SNDTIMEO`를 5초로 설정
- `server/http.c`에서 timeout 발생 시 `408 Request Timeout` 응답 반환
- `server/http.h`에 `HTTP_SOCKET_IO_TIMEOUT_SECONDS 5` 추가

### 2-2. 버킷 단위 락

- `sql_processor/table.h` / `sql_processor/table.c`에서 테이블을 `TABLE_BUCKET_COUNT` 기준 버킷으로 분리
- 각 버킷마다 `pthread_rwlock_t`를 사용
- `id % TABLE_BUCKET_COUNT` 기준으로 삽입/조회 경로가 나뉨

---

## 3. 실행한 테스트와 결과

### 3-1. SQL 단위 테스트

- 스크립트: `scripts/unit-tests.sh`
- 결과: `All unit tests passed.`

### 3-2. HTTP 통합 테스트

- 스크립트: `scripts/http_integration_test.sh`
- 결과: `HTTP integration tests passed.`

### 3-3. HTTP 프로토콜 엣지 케이스

- 스크립트: `scripts/http_protocol_edge_cases.sh`
- 결과: `HTTP protocol edge-case tests passed.`

### 3-4. HTTP timeout 테스트

- 스크립트: `scripts/http_timeout_test.sh`
- 결과: `HTTP timeout test passed.`
- 확인 내용:
  - 느린 요청은 `HTTP/1.1 408 Request Timeout`
  - 이후 정상 요청은 `HTTP/1.1 200 OK`

### 3-5. 버킷 락 스트레스 테스트

- 스크립트: `scripts/rwlock_stress_test.sh`
- 결과: `bucket lock stress test passed.`

### 3-6. Smoke test

- 스크립트: `scripts/tests/http/smoke-test.sh`
- 상태:
  - 스크립트 본문은 줄바꿈 호환성과 curl/python3 fallback을 넣어 정리했다.
  - 이 환경에서는 wrapper 실행보다 직접 본문을 호출하는 쪽이 안정적이었다.

---

## 4. 해석

- 타임아웃은 느린 클라이언트가 worker를 오래 점유하는 상황을 막는 데 유효했다.
- 버킷 단위 락은 기존 전역 테이블 락보다 경쟁 범위를 줄이면서도, 현재 테스트에서는 안정성을 유지했다.
- 테스트 스크립트는 대체로 정상 동작했지만, 일부 shell wrapper는 환경 차이 때문에 직접 본문 실행이 더 안정적이었다.

---

## 5. 결론

이번 변경은 단순히 "에러가 안 나는지"를 넘어서,

- 느린 I/O가 worker를 오래 붙잡는 문제
- 테이블 전체 락으로 인한 경합 문제

를 각각 분리해서 완화하는 방향으로 검증되었다.

