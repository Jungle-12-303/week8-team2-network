# HTTP 요청 파싱 정리함

## 목적

요청 라인을 읽고 body를 꺼내는 로직을 분리해서 정리한다.

## 범위

- request line 파싱함
- method/path 분리함
- header 읽기 기준 정리함
- `Content-Length` 처리함
- body 추출 기준 정리함

## 완료 기준

- 잘못된 입력을 어디서 거르는지 분명함
- 정상 요청과 비정상 요청의 경계가 보임
- 파싱 책임이 한 곳에 모임

## 테스트/검증

- 정상 `POST /query` 입력을 확인함
- 잘못된 method를 확인함
- body 없는 요청을 확인함
- 잘못된 header를 확인함

## 의존성

- `docs/plan/03-api-contract.md`
- `docs/plan/06-implementation-order.md`

## 메모

- 응답 코드 정리 카드와 함께 보면 좋다.
