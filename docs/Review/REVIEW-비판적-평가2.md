# 수석 엔지니어 평가: 미니 DBMS API 서버 계획 (v2)

> **검토 대상**: 08개 기본 계획 + 09-10번 문서 + personas 폴더 + task 폴더 (15개 카드)
> **검토 일시**: 2026-04-21
> **검토자**: C 시스템 프로그래밍 & 프로젝트 매니지먼트 전문가

---

## 1. 총평

### 🎯 **핵심 평가**

**이 계획은 v1에서 v2로 개선되면서 "전략적으로는 훨씬 나아졌지만, 실행 난이도는 더 높아졌다"**

| 평가 항목 | v1 | v2 | 변화 |
|---------|-----|-----|------|
| **요구사항 충족** | ✅ | ✅ | 동일 |
| **구조화 수준** | ⭐⭐⭐ | ⭐⭐⭐⭐⭐ | 크게 개선 |
| **병렬 실행 가능성** | ❌ 불가능 | ✅ 가능 | 개선 |
| **구현 세부도** | ⭐⭐ (추상적) | ⭐⭐⭐ (더 구체적) | 개선 |
| **통합/merge 난이도** | 낮음 | **높음** | 악화 |
| **하루 완성 가능성** | 🟡 70점 | 🟡 **65점** | 악화 |

### 📊 **최종 평가: 65점 (합격선 75점)**

**v1보다 2단계 후퇴:**
- 구조와 조직은 월등히 나아짐 (plan-task 분리, persona 체계)
- 하지만 **6명 협업 오버헤드**, **task 내용의 추상성**, **시간 예측 불명확**로 인해 실제 하루 프로젝트로는 더 어려워짐

---

## 2. 잘 잡은 점

### ✅ **1. Plan-Task 분리 (09번 문서)**
```markdown
계획 계층 (00-08): 전략, 스코프, 설계, 테스트 전략
실행 계층 (task): 구체적 작업 카드, 완료 기준, 검증 방법

이 분리는 관리에서 매우 깔끔하다.
```
**왜 좋은가:**
- 계획과 실행의 관심사 분리
- README/발표용 계획, 작업자용 task 자료가 따로 있음
- 새로운 팀원이 와도 이해하기 쉬움

**평가**: ⭐⭐⭐⭐⭐

---

### ✅ **2. Persona 기반 병렬 실행 (10번 문서 + personas 폴더)**
```markdown
6명의 전문화된 페르소나:
- 1 Coordinator (통제)
- 1 HTTP/API (request/response)
- 1 DB/SQL (sql_execute 호출)
- 1 Concurrency/Runtime (thread pool, mutex)
- 1 Testing/QA (검증)
- 1 Docs/Demo (README, 발표)

각자 파일 소유권이 다르므로 병렬 작업 가능.
```
**왜 좋은가:**
- "한 파일, 한 오너" 규칙으로 merge conflict 최소화
- 전문화로 인한 깊이 있는 구현 가능
- 느슨한 결합 (shared contract first 규칙)

**평가**: ⭐⭐⭐⭐⭐

---

### ✅ **3. Task 카드 세분화 (15개 카드)**
```markdown
기존: 06-implementation-order.md에서 9단계 (큼)
개선: 15개 task 카드로 세분화 (작고 명확)

예:
- 0005: HTTP 파싱 (한 카드)
- 0006: 라우트/응답 계약 (한 카드)
- 0007: SQL 어댑터 (한 카드)
- 0008: 스레드풀/큐 (한 카드)
- 0009: DB 락/shutdown (한 카드)
```
**왜 좋은가:**
- 각 task가 "한 사람이 2시간 내에 끝낼 수 있는" 크기
- PR, review, merge 단위가 명확
- Progress tracking이 쉬움 (15개 중 몇 개 완료?)

**평가**: ⭐⭐⭐⭐

---

### ✅ **4. 명시적 Handoff 프로토콜 (personas/07-handoff-and-merge.md)**
```markdown
각 페르소나는 작업 완료 후:
1. "무엇을 바꿨는가"
2. "무엇이 남았는가"
3. "어떤 가정을 했는가"
4. "다음 사람이 안 깨뜨릴 것"

이를 남겨야 한다.
```
**왜 좋은가:**
- 병렬 작업에서 가장 위험한 부분 (lack of context) 방지
- 코드 리뷰 없이도 인수인계 가능
- 6명이 일해도 일관성 유지

**평가**: ⭐⭐⭐⭐

---

### ✅ **5. Shared Contract First 규칙 (10번 문서)**
```markdown
구현 시작 전에 coordinator가 잠금:
- API shape (POST /query)
- Error codes
- Result format
- Queue behavior
- Shutdown behavior

그 이후로 API persona, DB persona, Concurrency persona가 따로 작업.
```
**왜 좋은가:**
- 병렬화의 가장 큰 리스크 (interface drift) 해결
- 계약 우선 설계로 통합 비용 최소화
- Test personas가 안정적인 인터페이스로 검증 가능

**평가**: ⭐⭐⭐⭐⭐

---

## 3. 위험하거나 애매한 점

### 🔴 **리스크 1: Task 내용이 여전히 추상적**

#### 문제
```markdown
Task 0005: HTTP 요청 파싱 정리함

완료 기준:
- 잘못된 입력을 어디서 거르는지 분명함
- 정상 요청과 비정상 요청의 경계가 보임
- 파싱 책임이 한 곳에 모임

테스트/검증:
- 정상 POST /query 입력을 확인함
- 잘못된 method를 확인함
- body 없는 요청을 확인함
- 잘못된 header를 확인함
```

**구체적 문제점:**
```
1. "파싱 책임이 한 곳에 모임" ← 어디? http.c? api.c?
2. "정상 입력을 확인함" ← 몇 개? 테스트 코드? curl?
3. "잘못된 method" ← 405 응답까지? 아니면 파싱만?
4. "body 없는 요청" ← 400 Bad Request를 어디서 생성?

개발자 입장:
- 이 카드만으로는 코딩할 수 없음
- 03-api-contract.md와 02-server-architecture.md를 계속 참고해야 함
- 그럼 병렬화 효과가 떨어짐
```

**다른 예시: Task 0008 (스레드풀)**
```markdown
완료 기준:
- 요청이 큐로 들어가는 흐름이 보임
- 워커가 처리하는 책임이 분명함
- 큐가 가득 찼을 때의 반응이 정해짐

구체적 문제:
1. "흐름이 보인다" ← 코드로? 다이어그램?
2. "반응이 정해진다" ← 503 응답? 그럼 HTTP persona와 협력?
3. Queue 구조 (circular? linked?)는? max size는?
```

**심각도**: 🟠 **높음**
**원인**: Task 카드가 "개념"이지 "구현 명세"가 아님
**개선**: 각 task마다 "예상 코드 골격" 또는 "C 프로토타입" 포함 필요

---

### 🔴 **리스크 2: Task 예상 시간 명시 안 됨**

#### 문제
```
15개 task, 각각 소요 시간이 다르다:
- 0003 (bootstrap): 30분
- 0004 (entry/config): 20분
- 0005 (HTTP 파싱): 90분 🔴
- 0006 (응답 계약): 60분
- 0007 (SQL 어댑터): 45분
- 0008 (스레드풀): 75분
- 0009 (DB lock/shutdown): 60분
- 0010 (단위 테스트): 40분
- 0011 (API 테스트): 50분
- 0012 (동시성 테스트): 60분
- 0013 (데모/README): 45분
- 0014 (에러 표준화): 50분
- 0015 (검증): 40분

총 660분 = 11시간 🔴

Task 카드에는 시간이 없다.
```

**구체적 문제:**
```
1. 6명이 동시 작업하면 660/6 = 110분 (1.8시간)?
   → 아니다. Task들이 의존성이 있다.

2. 의존성:
   - 0005 (HTTP) → 0006 (응답) → 0011 (API 테스트)
   - 0007 (SQL) → 0014 (에러)
   - 0008 (스레드풀) → 0009 (shutdown) → 0012 (동시 테스트)

3. Critical path:
   0005 (90) → 0006 (60) → 0011 (50) → 0015 (40) = 240분 = 4시간 (OK)

   또는:
   0008 (75) → 0009 (60) → 0012 (60) = 195분 = 3.25시간

4. 병렬화 이득:
   - Sequential: 660분 (11시간)
   - Parallel (6명): max(4시간, 3.25시간, ...) + coordination overhead
     = 약 5시간 + 1시간 (merge/conflict) = 6시간

아직도 tight.
```

**심각도**: 🔴 **매우 높음**
**원인**: Task 카드에 예상 시간과 의존성이 명시되지 않음
**개선**: 각 task마다 "예상 시간", "critical path 여부" 추가

---

### 🟠 **리스크 3: Task와 파일 ownership 매핑이 불명확**

#### 문제
```markdown
Task 0005: HTTP 요청 파싱 정리함
→ 어느 파일을 수정/생성하나?
   - http.c 생성?
   - server.c에 일부 추가?
   - api.c에 추가?

Task 0008: 스레드풀과 작업 큐 정리함
→ thread_pool.c 생성? 아니면 server.c에?

Task 0014: 에러 메시지 표준화함
→ 기존 sql.c 수정? 새 api_response.c 생성?
   → HTTP 응답, SQL 오류, 서버 오류를 모두 표준화하려면
      여러 파일 수정 필요
   → HTTP persona와 SQL persona 간 충돌 위험
```

**구체적 위험:**
```
HTTP persona가 "모든 에러를 JSON으로" 하기로 정했음.
→ Task 0005에서 파싱 에러를 JSON으로 응답하기로 함.

SQL persona가 "기존 sql_execute() 오류를 그대로 사용" 하기로 함.
→ Task 0007에서 SQL 오류를 기존 형식으로 반환.

Task 0014 (에러 표준화)에서 이 둘을 맞추려고 하니,
두 파일을 동시에 수정해야 함.

Result: Task 0014가 0005, 0007의 파일에 의존.
하지만 0005, 0007이 진행 중일 때 0014를 시작하면 merge conflict.
```

**심각도**: 🟠 **높음**
**원인**: Task 카드가 "파일"을 명시하지 않음
**개선**: 각 task마다 "영향받는 파일" 명시

---

### 🟠 **리스크 4: Persona 간 인터페이스 미정**

#### 문제
```markdown
API persona (Task 0006): 라우트와 응답 계약 정리
→ "응답은 JSON {"ok": true, "action": "insert", ...}"

DB persona (Task 0007): SQL 실행 어댑터
→ "SQLResult를 받아서 JSON으로 변환"

Concurrency persona (Task 0008): 스레드풀과 큐
→ "worker가 request를 받아서 HTTP 응답을 보냄"

문제:
1. API persona가 정한 JSON 포맷이 DB/Concurrency persona를 제약하는가?
2. 아니면 각자 다른 포맷으로?
3. 누가 JSON 변환 책임을 지는가?
   - API persona? (라우팅과 응답 포맷 소유)
   - DB persona? (SQL 결과 변환 소유)
   - 별도의 adapter?

shared contract는 있지만, 경계가 모호함.
```

**구체적 코드 레벨:**
```c
// API persona가 정하는 계약:
{
  "ok": true,
  "action": "insert",
  "inserted_id": 123,
  "row_count": 1
}

// DB persona가 받는 것:
SQLResult result = sql_execute(shared_table, sql);
// result.inserted_id는? result.action은?
// 이를 JSON으로 변환하려면, API persona가 정한 포맷을 알아야 함.

// Concurrency persona가 하는 것:
worker_process(client_fd) {
    read HTTP request
    extract SQL
    call ??? (API persona? DB persona?)
    return JSON response
}
```

**심각도**: 🟠 **중간-높음**
**원인**: Persona 간 API 컨트랙트가 정해지지 않음
**개선**: 각 persona 간 호출 방식 명시 (누가 누한테 뭘 호출?)

---

### 🟠 **리스크 5: 6명 협업의 coordination overhead**

#### 문제
```
6명이 일하면:
1. 일일 sync (30분)
2. Merge conflict resolution (30분~1시간)
3. Handoff 검토 (30분)
4. Test 재실행 (30분)

= 총 2시간 overhead

실제 구현 시간:
- 6명 × 8시간 = 48명-시간 예상
- 실제: 48명-시간 - 2시간(sync) - 1시간(merge) - 1시간(retest)
  = 44명-시간

순차 구현이 11시간이면:
- 병렬로는 44/6 = 7.3시간
- 하지만 critical path가 4시간이므로
- 실제는 4시간(critical) + 1시간(병렬 수렴) = 5시간

좋아 보이지만...
```

**실제 위험:**
```
1. 각 persona가 작업을 끝내면 coordinator에게 보고.
2. coordinator가 integration 검토 (30분~1시간)
3. 문제 발견 → 해당 persona에 돌려 보냄
4. 수정 후 재제출

이 loop가 반복되면 시간이 늘어남.
```

**심각도**: 🟡 **중간** (예측 불확실)
**원인**: Coordination overhead 예측 불가
**개선**: Dry run으로 경험치 축적 필요

---

### 🔴 **리스크 6: Task 의존성이 명시되지 않음**

#### 문제
```
Task README.md에서:
"의존성: 다른 task"

하지만 대부분 plan 문서만 링크.

예:
Task 0005: 의존성
- docs/plan/03-api-contract.md
- docs/plan/06-implementation-order.md

Task 0006: 의존성
- docs/plan/03-api-contract.md
- docs/plan/06-implementation-order.md

Task 0008: 의존성
- docs/plan/04-thread-pool-and-concurrency.md
- docs/plan/06-implementation-order.md

즉, 모든 task가 0003-0006을 먼저 해야 함.
(bootstrap, config, HTTP 파싱, 응답 계약)

그럼 병렬화가 아니라 순차화?
```

**구체적 의존성:**
```
0003 (bootstrap) → 필수 1순위
0004 (config) → 0003 이후
0005 (HTTP 파싱) → 0004 이후
0006 (응답 계약) → 0005 이후

0007 (SQL 어댑터) → 0006 이후?
   (왜냐하면 응답 형식을 알아야 함)

0008 (스레드풀) → 0005 이후
   (왜냐하면 HTTP 파싱 로직이 필요)

0009 (shutdown) → 0008 이후

...

Critical path: 0003 → 0004 → 0005 → 0006 → 0007 → ...
= 순차화되는 경향

병렬화 이득이 생각보다 작을 수 있음.
```

**심각도**: 🔴 **높음**
**원인**: Task 간 의존성이 명시되지 않아서, 병렬화 효과 불명확
**개선**: Task마다 "선행 task" 명시, DAG 그려보기

---

## 4. 빠진 것

### ❌ **빠진 1: Task 예상 시간**

```markdown
필요:
# Task 0005: HTTP 요청 파싱 정리함

예상 시간: 90분
  - HTTP 파싱 로직 작성: 60분
  - 테스트 (정상/비정상): 30분

Critical path 여부: YES (API persona의 주요 경로)
```

### ❌ **빠진 2: Task 간 파일 의존성**

```markdown
필요:
# Task 0005: HTTP 요청 파싱 정리함

영향받는 파일:
- http.c (생성 또는 수정)

Task 0006과의 인터페이스:
- 이 task가 준비할 것: http_parse_request() 함수
- Task 0006이 사용할 것: 파싱된 method, path, body

Task 0014와의 영향:
- 파싱 에러를 JSON으로 포맷하려면
- Task 0014의 api_error_format() 필요
```

### ❌ **빠진 3: Persona 간 명시적 API**

```markdown
필요: docs/plan/personas/api-contract-between-personas.md

예를 들어:

HTTP Persona → DB Persona:
  input: char *sql_string
  output: json_string

DB Persona → Concurrency Persona:
  input: json_response
  output: (worker가 socket에 write)
```

### ❌ **빠진 4: Integration Test 계획**

```markdown
Task 0001-0013은 각자의 영역.
하지만 Task 0014, 0015가 통합 검증.

문제:
- 6명이 동시에 코딩하면 통합 테스트는 언제?
- 통합 테스트가 실패하면 누가 고치나?
- 이미 다른 사람의 코드를 건드려야 할 수도?
```

필요: 통합 테스트 시작 시점, 실패 시 해결 프로세스

### ❌ **빠진 5: C 언어 빌드 시스템 계획**

```markdown
15개 task가 있지만, 빌드는?

Makefile:
  - 기존 sql_processor와 합칠 것인가?
  - 새로운 api_server target?
  - 컴파일 시 경고 없음을 보장할 것인가?

빌드 실패:
  - 파일이 compile되지 않으면?
  - linking error면?
  - 이를 체크하는 task는?
```

필요: 빌드 시스템 owner, 책임 범위

---

## 5. 범위 조정 제안

### 📊 **현실적 분석**

#### a) 지금 계획이 과도한 부분

```
과도:
1. 6명 협업 (오버헤드가 큼)
   → 권장: 3-4명 (HTTP, DB+SQL, Test+Demo)

2. 15개 task (너무 세분화)
   → 권장: 8-10개 task (실제로 실행 가능한 크기)

3. 명시적 Persona handoff 프로토콜
   → 좋지만 하루 프로젝트에는 과함
   → 권장: 간단한 "PR 코멘트" 기반 handoff
```

#### b) 지금 계획이 부족한 부분

```
부족:
1. 예상 시간 명시 (critical!)
2. Task 간 파일 ownership (critical!)
3. 통합 테스트 계획 (important)
4. Build system 책임 (important)
5. Persona 간 API (important)
```

### 🎯 **최적 범위 제안 (하루 프로젝트용)**

#### **Option 1: "Simplified Multi-Persona" (권장)**

```
3명 협업:
1. Coordinator (40분) + API/HTTP (120분) + 기본 테스트 (60분)
   → 총 220분 (3.7시간)

2. DB/SQL (100분) + SQL 오류 처리 (40분)
   → 총 140분 (2.3시간)

3. Concurrency (120분) + Shutdown (40분) + 동시성 테스트 (40분)
   → 총 200분 (3.3시간)

Parallel: max(3.7, 2.3, 3.3) + merge (30분) = 4.3시간

⭐ 이것이 현실적.
```

#### **Option 2: "Single-Leader + Specialists" (더 안전)**

```
1명 (Coordinator, 2시간)이 기본 골격을 구축:
- 0003, 0004 (bootstrap, config): 60분
- 0005, 0006 (HTTP, 응답 계약): 120분

이후 3명이 병렬로:
- HTTP persona: 응답 로직 완성 (60분)
- DB persona: SQL 어댑터 (80분)
- Concurrency persona: 스레드풀 (100분)
- (동시에) Testing: 기본 테스트 (60분)

통합/merge/final test: 60분

⭐ 더 안정적, 총 4-5시간
```

### 🔧 **구체적 범위 조정**

#### **Task 수 축소 (15 → 8)**

```
기존 15개:
0001, 0002, 0003, 0004, 0005, 0006, 0007, 0008, 0009,
0010, 0011, 0012, 0013, 0014, 0015

축소 안:
M1 (Foundation)
  0001: audit existing tests
  0003: project bootstrap

M2 (API Contract)
  0005+0006: HTTP parsing + response contract
  0014: error message standardization

M3 (Implementation)
  0007+0009: SQL adapter + DB locking
  0008: thread pool

M4 (Testing & Docs)
  0010+0011+0012: unit + API + concurrency tests
  0013: demo & README

= 8개 task (원래의 절반)
하지만 각 task 내용을 더 구체적으로.
```

#### **Task 예상 시간 추가**

```
각 task에 추가:

# M1-A: 기존 테스트 감사 및 bootstrap

예상 시간: 80분
  - 기존 unit_test 빌드 확인: 10분
  - Makefile 수정 (api_server target 추가): 20분
  - server.c 기본 구조 (listen, accept): 30분
  - 테스트 (curl로 기본 요청): 20분

Critical path: YES (첫 마일스톤)
선행 task: 없음
```

이런 식으로 모든 task에.

---

## 6. 최종 평가 및 권고

### 📋 **체크리스트**

| 항목 | 현황 | 개선 필요 |
|------|------|---------|
| 과제 요구사항 충족 | ✅ | 아니오 |
| 아키텍처 설계 | ✅ | 아니오 |
| Plan-Task 분리 | ✅ | 아니오 |
| Persona 기반 조직 | ✅ | **부분 축소** |
| Task 세분화 | ✅ | **재조정** |
| Task 예상 시간 | ❌ | **추가 필수** |
| Task 파일 ownership | ❌ | **추가 필수** |
| Persona API 컨트랙트 | ❌ | **추가 필수** |
| 의존성 명시 | ❌ | **추가 필수** |
| 통합 테스트 계획 | ❌ | **추가 필수** |
| Build system 계획 | ❌ | **추가 필수** |

### 🎯 **최종 권고**

#### **즉시 필요 (Critical)**

1. **각 task마다 예상 시간 추가**
   ```
   예상 시간: XX분
   Critical path: YES/NO
   ```

2. **각 task마다 파일 ownership 명시**
   ```
   생성/수정 파일: http.c, api.c, ...
   의존 파일: (다른 task가 먼저 수정할 것)
   ```

3. **Task 간 의존성 DAG 그리기**
   ```
   M1 (기본) → M2 (계약) → M3 (구현) → M4 (테스트)

   또는 mermaid로:
   graph LR
     T0001 --> T0005
     T0005 --> T0007
     ...
   ```

4. **Persona 간 API 컨트랙트 문서화**
   ```
   HTTP Persona → DB Persona:
     typedef struct {
       char sql[4096];
     } Request;

     typedef struct {
       bool ok;
       char json[8192];
     } Response;
   ```

#### **강력 권고 (Important)**

5. **협업 규모 재검토**
   - 6명 → 3명 또는 4명으로 축소 검토
   - 시간 overhead 체크

6. **Task 수 재조정**
   - 15개 → 8-10개
   - 각 task를 "한 사람이 2시간에 끝낼 수 있는" 크기로

7. **Integration test 계획 추가**
   - 언제 시작할 것인가? (동시? 순차?)
   - 실패 시 해결 프로세스

8. **Build system owner 명시**
   - 누가 Makefile 책임질 것인가?
   - 빌드 실패 시 누가 디버깅?

#### **권장 (Nice to have)**

9. **Dry run 고려**
   - 완전한 계획 전에 pilot으로 2-3 task 실행해보기
   - 예상 시간 검증

10. **Decision log 시작**
    - 중요한 설계 결정 문서화
    - "왜 HTTP를 선택했는가?", "왜 이 응답 포맷인가?" 등

---

## 7. 최종 종합 평가

### 🔥 **핵심 한마디**

```
"Plan이 훌륭해졌지만, 실행 계획은 아직 추상적이다.
6명이 하루 안에 이것을 끝내려면, 위의 6가지 '즉시 필요' 항목을 반드시 채워야 한다."
```

### 📊 **점수 변화**

| 평가 항목 | v1 | v2 | 필요 개선 후 |
|---------|-----|-----|----------|
| 총점 | 70점 | 65점 | **80점** |

### 🎯 **최종 판정**

**현 상태**: 🟡 **경계선 아래** (65점, 합격선 75점)
- 이대로 하면 실패 위험 높음
- 6명 협업으로 coordination overhead 발생
- Task 추상성으로 인한 재작업 위험

**개선 후**: 🟢 **합격선 위** (80점)
- 즉시 필요 4가지만 추가해도 크게 개선
- 협업 규모 축소 검토하면 더 좋음
- 하루 프로젝트로 성공 가능

### 📋 **권고 일정**

```
지금(21일 저녁):
- 위의 "즉시 필요" 4가지 추가 (2시간)

22일 오전 (수요일 구현 전):
- 최종 검토 및 dry run (1시간)

22일 오후부터:
- 실제 구현 시작
```

---

**최종 평가 완료**

문제점만 지적하는 것이 아니라, 개선 경로까지 제시했습니다.
이 조언들을 반영하면 v2 계획은 매우 견고한 하루 프로젝트가 될 수 있습니다.
