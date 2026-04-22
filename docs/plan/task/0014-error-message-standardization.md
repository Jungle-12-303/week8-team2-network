# 에러 메시지 표준화함

## 목적

HTTP, SQL, 서버 내부 에러 메시지의 형식을 맞춘다.

## 범위

- SQL error mapping 정리함
- HTTP status와 메시지 연결함
- 응답 body 포맷 통일함
- 로그와 사용자 응답을 구분함

## 완료 기준

- 같은 종류의 실패가 같은 형식으로 나감
- 사용자 메시지가 일관됨
- 디버깅에 필요한 정보가 빠지지 않음

## 테스트/검증

- 여러 실패 케이스의 메시지를 비교함
- body가 통일된 구조인지 확인함

## 의존성

- `docs/plan/03-api-contract.md`
- `docs/plan/07-test-and-quality-plan.md`

## 메모

- 기능 완성도 체감에 큰 영향을 준다.
