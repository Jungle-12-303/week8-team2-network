# Architecture Docs

이 디렉터리는 미니 DBMS API 서버의 최소 설계 산출물을 모아두는 공간이다.

## 문서 목록

- `api-spec.md`: endpoint, 요청/응답 형식, 상태 코드
- `request-flow.md`: 요청 처리 흐름, 모듈 경계, 락 전략

## 설계 원칙

- 하루 안에 구현 가능한 단순한 구조를 우선한다.
- TEAM7 엔진은 재사용하되, 서버 적합성을 위해 필요한 범위의 수정은 허용한다.
- 완전한 HTTP/1.1이 아니라 제한된 subset만 지원한다.
- 동시성은 coarse-grained read-write lock으로 먼저 안전하게 만든다.
