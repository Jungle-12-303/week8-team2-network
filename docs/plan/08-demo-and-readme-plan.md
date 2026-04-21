# 08. Demo and README Plan

## 코드 주석 원칙

데모와 README에 소개할 핵심 코드에는 한국어 주석을 친절하게 남깁니다. 발표자가 화면에서 코드를 열었을 때 API 요청이 어떤 함수들을 거쳐 DB 엔진까지 도달하는지, thread pool과 DB mutex가 왜 필요한지 바로 설명할 수 있어야 합니다.

이 문서는 목요일 오전 발표와 최종 README 구성을 위한 계획입니다.

## 발표 목표

4분 안에 다음 질문에 답할 수 있어야 합니다.

- 어떤 API 서버를 만들었는가.
- 기존 SQL 처리기와 B+Tree를 어떻게 재사용했는가.
- 스레드 풀은 어떤 흐름으로 요청을 처리하는가.
- 테스트로 무엇을 검증했는가.
- 다른 팀과 비교해 어떤 구현 포인트가 있는가.

## README 권장 구성

최종 README는 아래 순서로 정리합니다.

1. 프로젝트 소개.
2. 핵심 기능.
3. 전체 아키텍처.
4. API 명세.
5. 빌드 및 실행 방법.
6. curl 데모.
7. 테스트 방법과 결과.
8. 동시성 설계.
9. 기존 SQL 엔진과 B+Tree 활용 설명.
10. 한계와 개선 방향.

## 데모 시나리오

### 1. 서버 실행

```bash
make api_server
./api_server 8080
```

### 2. INSERT 요청

```bash
curl -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "INSERT INTO users VALUES ('Alice', 20);"
```

보여줄 점:

- 외부 클라이언트가 API로 DB에 row를 추가합니다.
- 응답에 inserted id가 표시됩니다.

### 3. SELECT 요청

```bash
curl -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "SELECT * FROM users;"
```

보여줄 점:

- API 응답으로 DBMS 결과를 확인합니다.
- 내부적으로 기존 SQL 처리기와 B+Tree가 사용됩니다.

### 4. 조건 조회

```bash
curl -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "SELECT * FROM users WHERE id >= 1;"
```

보여줄 점:

- `id` 조건은 B+Tree 인덱스 경로를 활용합니다.

### 5. 동시 요청

```bash
seq 1 20 | xargs -n1 -P8 -I{} curl -s -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "INSERT INTO users VALUES ('User{}', {});"
```

보여줄 점:

- worker thread들이 queue에서 작업을 가져가 처리합니다.
- 공유 DB는 mutex로 보호됩니다.

## 발표 설명 흐름

1. “이번 과제는 기존 DB 엔진을 API 서버로 감싸는 것이 목표였습니다.”
2. “서버는 accept loop와 thread pool을 분리했습니다.”
3. “worker는 HTTP body에서 SQL을 꺼내 `sql_execute()`로 넘깁니다.”
4. “기존 Table과 B+Tree는 공유 자료구조이므로 DB mutex로 보호했습니다.”
5. “테스트는 기존 SQL 단위 테스트, API curl 테스트, 동시 요청 테스트로 나눴습니다.”

## 개선 방향

- `pthread_rwlock_t`로 SELECT와 INSERT 잠금 분리.
- JSON request body 지원.
- 여러 table 또는 더 많은 SQL 문법 지원.
- graceful shutdown.
- 간단한 benchmark와 thread pool 크기별 비교.
