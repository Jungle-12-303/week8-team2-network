# HyeYeon 브랜치 vs exp1 브랜치 과제 기준 비교 리포트

작성일: 2026-04-21

## 범위

이 문서는 과제 가이드라인 기준으로 `HyeYeon` 브랜치와 `exp1` 브랜치를 비교한 결과를 정리한다.

평가에서 제외한 항목:

- 발표
- README 작성

평가 대상:

- API 서버 구현 완성도
- 내부 DB 엔진 연동
- 스레드 풀 및 동시성 처리
- 에러 처리 및 엣지 케이스
- 테스트 및 검증 품질
- 구조적 완성도와 확장성

## 비교 기준 요약

과제 가이드라인에서 가장 중요한 부분은 다음과 같다.

- 외부 클라이언트가 사용할 수 있는 API 서버인가
- 요청마다 스레드를 할당해 SQL을 병렬로 처리하는가
- 기존 SQL 처리기와 B+ 트리 인덱스를 활용하는가
- 동시성 이슈를 얼마나 안전하게 다루는가
- 단위 테스트와 기능 테스트로 검증했는가

## 테스트 및 검증 방법

실제 비교를 위해 다음을 확인했다.

1. `HyeYeon` 브랜치 Docker 이미지 빌드
2. `HyeYeon` 브랜치 서버 실행 후 HTTP 요청 직접 전송
3. `exp1` 브랜치 Docker Compose 기반 빌드 및 smoke test 실행
4. `exp1` 브랜치 서버 실행 후 HTTP 요청 직접 전송

확인한 대표 시나리오:

- 정상 insert
- 정상 select
- 잘못된 SQL
- 잘못된 라우트
- health check
- 테스트 스크립트 실행 가능 여부

## 점수표

총점은 60점 만점으로 잡았다.

| 항목 | 배점 | HyeYeon | exp1 | 우세 |
|---|---:|---:|---:|---|
| 과제 적합성 / API 계약 | 10 | 10 | 5 | HyeYeon |
| 내부 DB 엔진 연동 | 10 | 9 | 7 | HyeYeon |
| 동시성 / 스레드풀 | 10 | 6 | 9 | exp1 |
| 에러 처리 / 엣지 케이스 | 10 | 9 | 6 | HyeYeon |
| 테스트 / 검증 품질 | 10 | 7 | 4 | HyeYeon |
| 구조 / 확장성 | 10 | 7 | 8 | exp1 |
| 합계 | 60 | 48 | 39 | HyeYeon |

## HyeYeon 브랜치 분석

### 강점

`HyeYeon` 브랜치는 과제 요구사항과 가장 직접적으로 맞닿아 있다. 핵심 이유는 외부 API가 `POST /query` 중심으로 동작하면서, 클라이언트가 SQL을 그대로 전송하는 과제형 인터페이스를 충실히 따르기 때문이다.

실제 확인 결과:

- `POST /query` + `INSERT`가 정상 동작했다.
- `POST /query` + `SELECT`가 정상 동작했다.
- 문법이 깨진 SQL도 구조화된 JSON 에러로 반환됐다.
- `GET /health` 같은 별도 라우트는 없지만, 과제 핵심인 SQL API 자체는 명확하다.

동시성 측면에서는 `server/api.c`에서 DB 접근을 mutex로 감싸고 있어 읽기 요청까지 직렬화되는 한계가 있다. 다만 이 구조는 단순하고 안전하며, 적어도 동시성 버그를 피하는 쪽으로 보수적으로 설계되어 있다.

에러 응답 품질도 좋다. HTTP 수준의 에러와 SQL 수준의 에러를 분리해서 다루며, `syntax_error`, `query_error`, `not_found`, `internal_error` 같은 분류가 잘 보인다.

### 약점

가장 큰 약점은 읽기 병렬성이 부족하다는 점이다. SELECT 요청까지 하나의 mutex로 막기 때문에, 과제에서 강조한 동시성 최적화 관점에서는 아쉬움이 있다.

또한 디렉터리 구조가 `server/`와 `sql_processor/` 중심이라 직관적이지만, 기능 단위로 더 세분화된 구조는 아니다.

## exp1 브랜치 분석

### 강점

`exp1` 브랜치는 구조적으로 더 잘 나뉘어 있다.

- `src/server/server.c`: HTTP 요청과 라우팅
- `src/db/db_adapter.c`: DB 접근 및 락 정책
- `src/concurrency/thread_pool.c`: worker 관리
- `src/concurrency/job_queue.c`: 큐 구현

특히 `db_adapter.c`에서 `pthread_rwlock_t`를 사용해 읽기/쓰기 락을 분리한 점은 매우 좋다. 읽기 요청이 많은 DB 서버에서 설계적으로 더 우수하다.

실제 확인 결과:

- `GET /health`는 정상 동작했다.
- `POST /users`와 `GET /users`, `GET /users/1`는 동작했다.
- `/query`는 지원하지 않아 404가 반환됐다.

### 약점

가장 큰 약점은 과제 API와의 불일치다. 과제는 SQL API 서버인데, exp1은 REST 리소스형 API(`/users`) 중심이라 요구사항과 어긋난다.

또한 자동 테스트가 깨졌다. `docker compose run --rm test` 실행 시 `tests/smoke_test.sh`가 CRLF 문제로 실패했다.

실행 로그 상 핵심 실패는 다음과 같았다.

- `tests/smoke_test.sh: line 2: $'\r': command not found`
- `set: pipefail\r: invalid option name`

즉, 서버 로직은 일부 정상이어도, 검증 파이프라인이 바로 무너진다. 과제 제출물 관점에서는 상당히 큰 감점 요인이다.

## 항목별 상세 비교

### 1. 과제 적합성 / API 계약

`HyeYeon`이 압도적으로 유리하다.

- HyeYeon: `POST /query`로 SQL을 직접 받는다.
- exp1: `/users` CRUD 형태라 SQL API 과제와 다르다.

과제에서 중요하게 보는 “외부 클라이언트가 DBMS 기능을 사용할 수 있어야 한다”는 요구를 기준으로 보면, HyeYeon이 훨씬 정석적인 해석이다.

### 2. 내부 DB 엔진 연동

둘 다 기존 엔진을 사용하지만, 방식이 다르다.

`HyeYeon`은 HTTP 요청이 들어오면 바로 SQL 실행기로 전달하는 형태라 흐름이 단순하다.

`exp1`은 DB 어댑터를 둬서 구조를 분리했기 때문에 유지보수성은 좋다. 다만 API가 SQL을 직접 드러내지 않아서, 과제의 “SQL 처리기와 B+ 트리 인덱스를 그대로 활용”한다는 느낌은 HyeYeon이 더 강하다.

### 3. 동시성 / 스레드풀

이 항목은 `exp1`이 더 낫다.

이유:

- 읽기와 쓰기를 분리한 RW lock을 사용한다.
- 구조상 SELECT가 동시에 여러 개 처리될 가능성이 있다.
- job queue와 worker pool의 책임이 명확하다.

반면 `HyeYeon`은 단순 mutex로 DB 전체를 잠그므로 동시성 성능이 떨어진다.

다만 HyeYeon의 장점은 구현 안정성이다. 동시성 최적화는 덜하더라도, 로직이 단순해 버그 가능성은 낮다.

### 4. 에러 처리 / 엣지 케이스

`HyeYeon`이 더 좋다.

실제 응답 기준으로:

- 잘못된 HTTP 메서드에 대해 405를 반환한다.
- 잘못된 라우트에 대해 404를 반환한다.
- SQL 문법 오류를 JSON 에러로 돌려준다.
- 큐 포화 시 503을 반환한다.

`exp1`도 기본적인 400/404 처리와 데이터 검증은 있으나, 과제형 SQL API 관점에서의 에러 체계는 HyeYeon이 더 풍부하고 일관적이다.

### 5. 테스트 / 검증 품질

`HyeYeon`이 더 낫다.

이유:

- Docker 빌드가 성공했다.
- 실제 HTTP 요청으로 insert/select/error 케이스를 확인했다.
- 서버가 과제 핵심 시나리오를 만족한다.

`exp1`은 서버 자체 일부는 정상인데도, smoke test가 스크립트 인코딩 문제로 바로 실패한다. 이건 단순한 형식 문제가 아니라, 팀이 “검증 가능한 상태”를 유지하고 있는지에 대한 신뢰도에도 영향을 준다.

### 6. 구조 / 확장성

이 항목은 `exp1`이 더 우세하다.

좋은 점:

- 서버, DB, 스레드풀, 큐가 분리돼 있다.
- 읽기 전용 요청 최적화 포인트가 명확하다.
- 운영용 health endpoint가 있다.

즉, 장기적으로 발전시키기 좋은 기초 구조는 exp1 쪽이 더 탄탄하다.

## 결론

과제 제출 관점에서는 `HyeYeon` 브랜치를 더 높게 평가하는 것이 타당하다.

이유는 단순하다.

1. 과제 API 계약에 더 잘 맞는다.
2. 실제 SQL 흐름이 더 직관적이다.
3. 에러 응답이 풍부하다.
4. 테스트가 실제로 통과했다.

반면 `exp1`은 구조와 동시성 설계는 더 세련됐지만, 과제 요구사항과의 정합성이 낮고 테스트가 깨져 있어서 감점이 크다.

## 최종 판단

- 제출 안정성: `HyeYeon`
- 구조적 세련미: `exp1`
- 과제 기준 최종 승자: `HyeYeon`

## 참고한 파일

- `server/server.c`
- `server/api.c`
- `server/thread_pool.c`
- `src/server/server.c`
- `src/db/db_adapter.c`
- `src/concurrency/thread_pool.c`
- `tests/smoke_test.sh`
