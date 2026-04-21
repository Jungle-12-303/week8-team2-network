# 11. Implementation Policies

## 목적

이 문서는 실제 구현 중에 흔들리기 쉬운 정책을 미리 정리한다.

시간 추정은 제외한다.
이 프로젝트는 Codex 기반 병렬 실행을 전제로 하므로, 이 문서는 "얼마나 걸리나"보다 "무엇을 어떻게 고정할 것인가"에 집중한다.

## HTTP I/O 정책

- socket fd를 기준으로 raw `recv()` 루프를 사용한다
- `fdopen()`과 `fgets()`는 기본 정책으로 쓰지 않는다
- 요청은 partial read를 전제로 누적 읽기한다
- `POST /query`만 기본 지원한다
- `Content-Length`가 실제 body 길이와 맞지 않으면 `400 Bad Request`로 처리한다
- chunked transfer encoding과 keep-alive는 MVP에서 제외한다

## SQL 오류 정책

- SQL 오류는 HTTP 파싱 오류와 분리한다
- HTTP 요청이 정상이고 SQL 실행만 실패한 경우에는 `200 OK` + JSON 실패 응답을 기본값으로 둔다
- `error_code`, `sql_state`, `error_message`는 가능한 한 `SQLResult`에서 그대로 활용한다
- 엔진이 세부 정보를 덜 주면 API 계층에서 최소 필드를 보강한다

## JSON 직렬화 정책

- `SQLResult.records`는 포인터 배열이므로, 응답 생성 중에 참조 안정성을 확보해야 한다
- MVP에서는 DB lock 안에서 SQL 실행과 JSON 직렬화를 끝내는 방식을 기본으로 둔다
- 응답 문자열은 완성한 뒤에 socket write를 수행한다
- 메모리 할당 실패는 500 계열 오류로 처리한다

## 종료 정책

- graceful shutdown의 상세 방식은 [`12-graceful-shutdown-strategy.md`](12-graceful-shutdown-strategy.md)에서 확정한다
- 이 문서에서는 종료 정책의 공통 원칙만 고정한다
- 새 요청 수락은 종료 신호 이후 중단한다
- worker는 종료 상태를 확인한 뒤 안전하게 빠져나가도록 한다
- `pthread_join()`으로 정리한다

## 큐 정책

- job queue는 circular buffer로 둔다
- queue가 가득 차면 `503 Service Unavailable`을 반환한다
- 503 응답은 accept loop 또는 공통 helper에서 생성한다
- queue mutex는 짧게만 잡는다

## 빌드 정책

- 기존 `sql_processor`의 빌드는 깨지지 않아야 한다
- 새 서버 target은 기존 엔진과 분리된 이름으로 둔다
- 경고 없는 빌드를 목표로 한다
- Build 책임은 코디네이터가 최종 확인한다

## 통합 정책

- 페르소나별 작업은 작업 카드 단위로 병합한다
- 인터페이스 계약이 바뀌면 관련 카드와 문서를 동시에 갱신한다
- 통합 테스트는 API, SQL, 동시성을 한 번에 확인하는 단계로 둔다
- 실패 시 구현 먼저가 아니라 계약부터 다시 본다

## 검증 정책

- 단위 테스트는 기존 `sql_processor`를 우선 검증한다
- API 테스트는 HTTP 파싱, 응답 코드, JSON 형태를 확인한다
- 동시성 테스트는 queue full, 반복 insert, shutdown을 확인한다
- 데모는 성공 경로만이 아니라 실패 경로도 최소 하나 보여준다
