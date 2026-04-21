# Reviewer 메모

## 검토 범위

- PM 기능 문서
- Architect 설계 문서
- MVP 서버 구현 코드
- QA smoke test 결과

## 확인된 점

- 플랜의 최소 구현 범위를 지키고 있다.
- TEAM7 엔진은 `third_party/team7_engine`으로 포함되고, 서버는
  `db_adapter`를 통해서만 접근한다.
- 메인 스레드와 worker의 책임 분리가 문서와 구현에서 일치한다.
- `make test`가 통과해 기본 실행 경로는 검증됐다.

## 남은 리스크

### 1. JSON 파싱이 최소 문자열 탐색 기반이다

- 영향도: 중간
- 설명: 현재 `POST /users` body 파싱은 범용 JSON 파서가 아니라 최소 패턴 탐색 방식이다.
- 후속 조치: escape 문자, 필드 순서 변화, 공백 변형에 대한 테스트를 더 늘릴 필요가 있다.

### 2. graceful shutdown이 없다

- 영향도: 중간
- 설명: 현재는 최소 구현 범위대로 동작하며, 정상 종료 시그널 처리와 큐 drain은 없다.
- 후속 조치: 발표 후 확장 항목으로 분리하는 것이 맞다.

### 3. QA 범위가 smoke test 중심이다

- 영향도: 중간
- 설명: 기본 endpoint는 검증됐지만 malformed request와 혼합 동시성 케이스는 자동화가 부족하다.
- 후속 조치: `test-cases.md` 기준으로 수동 검증 또는 추가 스크립트를 보강해야 한다.

## 병합 전 확인 항목

- README의 실행 명령과 실제 바이너리 이름이 일치하는지 확인
- `make clean && make && make test` 재실행
- 발표 데모 순서를 README와 맞춤
