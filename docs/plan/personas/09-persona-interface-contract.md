# Persona Interface Contract

## 목적

페르소나별 작업이 병렬로 진행되더라도 서로 무엇을 주고받는지 분명하게 정리한다.

## 기본 원칙

- 각 페르소나는 자기 책임 영역의 결과를 명확한 형식으로 넘긴다
- 계약을 바꾸는 일은 코디네이터가 먼저 승인한다
- 카드에 인터페이스가 없으면 병렬 작업을 시작하지 않는다

## HTTP / API → DB / SQL

HTTP / API 페르소나는 다음 형태의 입력을 DB / SQL 페르소나에게 넘긴다.

- 정제된 SQL 문자열
- 요청 파싱 결과
- 요청이 유효한지 여부
- 파싱 실패 시 HTTP 오류 정보

DB / SQL 페르소나는 다음을 돌려준다.

- `SQLResult`
- 실행 상태
- 오류 코드
- SQL state
- 오류 메시지
- 필요한 경우 row 목록

## DB / SQL → HTTP / API

DB / SQL 페르소나는 HTTP 응답 포맷 자체를 책임지지 않는다.

대신 다음 정보를 제공한다.

- `status`
- `action`
- `inserted_id`
- `row_count`
- `records`
- `error_code`
- `sql_state`
- `error_message`

HTTP / API 페르소나는 이를 JSON 응답으로 바꾼다.

## HTTP / API ↔ 동시성 / 런타임

동시성 / 런타임 페르소나는 다음을 책임진다.

- client fd를 받아 job queue에 넣는다
- worker thread에 job을 배분한다
- queue full 상황을 감지한다
- 종료 신호를 worker에게 전달한다

HTTP / API 페르소나는 다음을 책임진다.

- 요청 파싱
- 응답 문자열 완성
- 에러 응답 생성

## 동시성 / 런타임 ↔ 테스트 / QA

테스트 / QA 페르소나는 다음 조건을 검증한다.

- queue full 시 `503`이 나오는지
- 종료 시 worker가 안전하게 빠지는지
- 동시 요청에서 race condition이 없는지

동시성 / 런타임 페르소나는 테스트가 쓸 수 있는 관측 포인트를 남긴다.

## 코디네이터 ↔ 나머지 페르소나

코디네이터는 다음을 고정한다.

- task 카드 소유권
- 파일 경계
- 인터페이스 형식
- build target
- 통합 순서

나머지 페르소나는 코디네이터가 고정한 계약을 따라 구현한다.
