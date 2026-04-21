# QA 검증 결과

## 실행 정보

- 날짜: 2026-04-21
- 브랜치: `exp1`
- 검증 기준 문서: [test-cases.md](/Users/choeyeongbin/week8-team2-network/project/qa/test-cases.md)
- 실행 명령:

```bash
make test
```

## 요약

- 결과: 통과
- 범위: MVP 핵심 smoke test
- 확인된 endpoint:
  - `GET /health`
  - `POST /users`
  - `GET /users`
  - `GET /users/{id}`

## 상세 결과

### TC-SMOKE-001 `GET /health`

- 결과: 통과
- 확인 내용: 서버가 `200 OK`와 JSON 응답을 반환했다.

### TC-SMOKE-002 `POST /users`

- 결과: 통과
- 확인 내용: 유효한 JSON body로 사용자 생성이 성공했다.

### TC-SMOKE-003 `GET /users`

- 결과: 통과
- 확인 내용: 생성된 사용자 목록이 JSON으로 반환됐다.

### TC-SMOKE-004 `GET /users/1`

- 결과: 통과
- 확인 내용: 생성된 사용자 한 건 조회가 성공했다.

## 비고

- 이번 결과는 smoke test 기준이다.
- malformed request, 동시성 혼합 요청, 경계 입력 검증은 별도 수동 테스트가 더 필요하다.
