# MVP 리뷰 - 미니 DBMS API 서버

## 📋 요구사항 체크리스트

### 핵심 요구사항 (필수)

| 항목 | 상태 | 비고 |
|------|------|------|
| 미니 DBMS - API 서버 구현 | ✅ | HTTP 서버 구현 완료 |
| 외부 클라이언트에서 DBMS 기능 사용 | ✅ | POST /query 엔드포인트 동작 확인 |
| 스레드 풀(Thread Pool) 구성 | ✅ | 4개 워커, 16개 큐 용량 |
| 멀티스레드 SQL 요청 병렬 처리 | ✅ | 스레드 풀 + 뮤텍스 기반 동시성 처리 |
| SQL 처리기 & B+ 트리 인덱스 활용 | ✅ | sql_processor 패키지 통합 |
| C 언어 구현 | ✅ | 순수 C로 구현 |

---

## 🎯 중점 포인트 분석

### 1️⃣ 멀티 스레드 동시성 이슈

**구현 현황:**
```c
// server.c: DB 뮤텍스
pthread_mutex_t db_mutex;

// thread_pool.c: 작업 큐 동기화
pthread_mutex_lock(&pool->mutex);
while (!pool->stop_requested && pool->size == 0) {
    pthread_cond_wait(&pool->cond, &pool->mutex);
}
```

**✅ 평가:**
- **장점:**
  - `pthread_mutex_t`로 DB 접근 보호
  - 조건 변수(`pthread_cond_t`)로 효율적 워커 대기
  - Critical section 최소화 (뮤텍스 범위 적절)
  
- **개선 여지:**
  - ⚠️ **동시성 테스트 부재**: smoke_test.sh에 순차 테스트만 있음
  - 실제 동시 요청 시 데이터 무결성 검증 필요
  - 스트레스 테스트 (수십~수백 개 동시 요청) 미실시

**추천 액션:**
```bash
# 동시 요청 테스트 예시
for i in {1..20}; do
  curl -X POST http://localhost:8080/query \
    -H "Content-Type: text/plain" \
    --data "INSERT INTO users VALUES ('User$i', $((20+i)));" &
done
wait

# 최종 행 개수 확인으로 데이터 무결성 검증
curl -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "SELECT * FROM users;" | jq '.row_count'
```

---

### 2️⃣ 내부 DB 엔진과 외부 API 서버 사이 연결

**아키텍처:**
```
┌─────────────────────────────────────┐
│   HTTP 클라이언트 (curl, 브라우저)  │
└────────────────┬────────────────────┘
                 │
          POST /query (JSON)
                 │
┌────────────────▼────────────────────┐
│     HTTP Server (server.c)           │
│  ├─ 요청 파싱 (http.c)              │
│  └─ 스레드 풀 (thread_pool.c)       │
└────────────────┬────────────────────┘
                 │
        API 핸들러 (api.c)
                 │
┌────────────────▼────────────────────┐
│   SQL 처리기 (sql_processor/)        │
│  ├─ SQL 파서 (sql.c)                │
│  ├─ 테이블 관리 (table.c)           │
│  └─ B+ 트리 인덱스 (bptree.c)       │
└────────────────┬────────────────────┘
                 │
          메모리 기반 DB
```

**✅ 평가:**
- **장점:**
  - 명확한 계층 분리 (HTTP ↔ API ↔ SQL Engine)
  - Raw socket 파싱으로 의존성 최소화
  - JSON 응답 포맷 표준화
  
- **개선 여지:**
  - ⚠️ **에러 처리 상세도**: 현재 JSON 에러만 반환
  - SQL 문법 오류 시 상세 메시지 부족
  - HTTP 상태 코드 미분화 (모두 200 OK?)

**추천 액션:**
```c
// 에러 응답 개선 예시
{
  "ok": false,
  "status": "syntax_error",
  "error_code": 1001,
  "sql_state": "42000",
  "message": "Unexpected token at position 15: 'FORM'"
}
```

---

### 3️⃣ API 서버 아키텍처

**구성 요소:**

| 모듈 | 역할 | 라인 수 |
|------|------|--------|
| `server.c` | 소켓 관리, 메인 루프 | 260 |
| `thread_pool.c` | 워커 스레드 관리 | 151 |
| `http.c` | HTTP 요청/응답 처리 | 288 |
| `api.c` | SQL 실행 + JSON 직렬화 | 177 |
| `json_util.c` | JSON 버퍼 관리 | 200 |

**✅ 평가:**
- **강점:**
  - 모듈식 설계로 테스트 용이
  - 의존성 최소화 (libc만 사용)
  - 메모리 누수 방지 (정리 함수 완비)
  
- **개선 여지:**
  - ⚠️ **연결 제한 없음**: 동시 연결 수 제한 기능 부재
  - 큐 오버플로우 시 요청 버림 (거절 응답 미발송)
  - Graceful degradation 미구현

**추천 액션:**
```c
// thread_pool_submit 반환값 처리 강화
if (!thread_pool_submit(&pool, client_fd)) {
    // 큐 가득참 → 503 Service Unavailable 응답
    http_send_response(client_fd, 503, "application/json", 
        "{\"ok\":false,\"message\":\"Server busy\"}");
    close(client_fd);
}
```

---

## 💡 기능 검증 결과

### 테스트한 SQL 기능

| SQL 쿼리 | 결과 | 응답 시간 |
|---------|------|---------|
| `INSERT INTO users VALUES ('Alice', 20);` | ✅ | <10ms |
| `SELECT * FROM users;` | ✅ | <10ms |
| `SELECT * FROM users WHERE age > 20;` | ✅ | <10ms |

**응답 포맷 (성공):**
```json
{
  "ok": true,
  "action": "insert",
  "inserted_id": 1,
  "row_count": 1
}
```

**응답 포맷 (조회):**
```json
{
  "ok": true,
  "action": "select",
  "row_count": 1,
  "rows": [
    {"id": 1, "name": "Alice", "age": 20}
  ]
}
```

---

## 🚀 추가 구현 요소 (차별점)

현재는 기본 기능만 구현된 상태입니다. 다른 팀과의 차별화를 위해 다음을 추천합니다:

### A. 성능 최적화
- [ ] 커넥션 풀링 (재사용 가능한 연결)
- [ ] 쿼리 캐싱 (자주 사용되는 SELECT 결과 캐시)
- [ ] 배치 처리 (여러 SQL을 한 트랜잭션에서 실행)

### B. 고급 기능
- [ ] 트랜잭션 (BEGIN, COMMIT, ROLLBACK)
- [ ] 사용자 인증 (토큰 기반)
- [ ] 쿼리 로깅 (모든 요청 기록)
- [ ] 데이터 백업/복구

### C. 안정성 강화
- [ ] 요청 타임아웃 관리
- [ ] 메모리 사용량 모니터링
- [ ] 장시간 연결 종료 (keep-alive 제한)
- [ ] 잘못된 요청 필터링 (SQL injection 기본 방어)

### D. 개발자 경험
- [ ] 상세한 에러 메시지
- [ ] 쿼리 실행 시간 정보
- [ ] 서버 상태 엔드포인트 (`GET /health`)

---

## 🎓 핵심 구현 이해도

**질문:** 다음을 설명할 수 있나요?

1. ✅ **스레드 풀이 작동하는 원리?**
   - Worker 스레드가 condition variable으로 대기
   - 요청 도착 시 뮤텍스로 보호된 큐에 추가
   - Worker가 깨어나 작업 처리

2. ✅ **DB 뮤텍스는 어디에 필요한가?**
   - `api_handle_query()` 호출 전후로 DB 접근 보호
   - 여러 워커가 동시에 같은 테이블 수정 방지

3. ✅ **HTTP 요청을 어떻게 파싱하나?**
   - Content-Length 헤더로 바디 크기 확인
   - 정해진 크기만큼 읽어서 SQL 쿼리 추출

4. ✅ **JSON 응답은 어떻게 생성하나?**
   - JsonBuffer 구조체로 동적 문자열 버퍼 관리
   - SQL 결과를 순회하며 JSON 포맷으로 직렬화

---

## 📊 최종 평가

| 평가 항목 | 점수 | 의견 |
|---------|------|------|
| 요구사항 충족도 | ⭐⭐⭐⭐⭐ | 모든 필수 요구사항 구현 완료 |
| 코드 품질 | ⭐⭐⭐⭐ | 명확한 구조, 의존성 최소 |
| 동시성 처리 | ⭐⭐⭐⭐ | 정확한 뮤텍스/CV 사용, 테스트 필요 |
| 테스트 완정도 | ⭐⭐⭐ | 기본 테스트만 있음, 동시성 테스트 부재 |
| 에러 처리 | ⭐⭐⭐ | 기본적이지만 개선 여지 있음 |
| **종합** | **⭐⭐⭐⭐** | **MVP 완성도 높음, 고도화 가능** |

---

## 🎬 다음 단계

1. **동시성 테스트 추가** (필수)
   - 20~50개 동시 요청으로 검증
   - 데이터 무결성 확인

2. **에러 처리 개선** (권장)
   - SQL 에러 메시지 상세화
   - HTTP 상태 코드 다양화

3. **차별화 기능 추가** (선택)
   - A, B, C, D 중 1~2개 선택 후 구현

