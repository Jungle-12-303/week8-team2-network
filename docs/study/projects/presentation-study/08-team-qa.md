# 8단계. 팀원별 Q&A 백업 나누기

## 목표

대표 발표자가 전체 흐름을 말하고, 팀원들이 API/HTTP, 동시성, DB, 테스트 영역별 Q&A를 백업할 수 있게 역할을 나눕니다.

## 예상 소요 시간

- 30~40분

## 역할 구조

- [ ] 대표 발표자: 전체 구조, 요청 흐름, 데모
- [ ] API/HTTP 백업: endpoint, HTTP parser, status code
- [ ] 동시성 백업: thread pool, queue, mutex, condition variable
- [ ] DB 백업: SQL 처리기, Table, B+Tree
- [ ] 테스트 백업: unit test, smoke test, concurrency test

## 8-1. 대표 발표자 역할

담당 범위:

- [ ] 프로젝트 한 문장 소개
- [ ] 전체 아키텍처
- [ ] 요청 처리 흐름
- [ ] 터미널 데모 진행
- [ ] 한계와 개선 방향

반드시 말할 흐름:

```text
client -> accept -> queue -> worker -> HTTP parser -> api_handle_query -> sql_execute -> JSON response
```

대표 발표자 체크 질문:

- [ ] 이 프로젝트를 한 문장으로 설명하면?
- [ ] 왜 API 서버를 붙였는가?
- [ ] 요청 하나가 들어오면 어떤 순서로 처리되는가?
- [ ] 동시성은 어떤 방식으로 처리되는가?
- [ ] 현재 한계와 개선 방향은 무엇인가?

대표 발표자 답변 템플릿:

```text
기존 REPL 기반 SQL 엔진을 C HTTP API 서버로 감싸 외부 클라이언트가 POST /query로 SQL을 실행할 수 있게 만든 프로젝트입니다.
```

```text
메인 스레드는 accept한 client fd를 thread pool queue에 넣고, worker가 HTTP 요청을 읽은 뒤 api_handle_query를 통해 기존 sql_execute를 호출합니다.
```

## 8-2. API/HTTP 백업 역할

담당 파일:

- [ ] `server/http.c`
- [ ] `server/http.h`
- [ ] `server/server.c`
- [ ] `server/api.c`

담당 개념:

- [ ] `POST /query`
- [ ] plain text SQL body
- [ ] `Content-Length`
- [ ] HTTP request line
- [ ] HTTP response header
- [ ] 400, 404, 405, 413, 500, 503
- [ ] SQL 오류 JSON과 HTTP 오류 JSON의 차이

답변해야 할 질문:

- [ ] 왜 `POST`만 지원하나요?
- [ ] 왜 `/query` 하나만 있나요?
- [ ] `Content-Length`는 왜 필요한가요?
- [ ] body가 너무 크면 어떻게 되나요?
- [ ] 잘못된 path는 어떻게 처리하나요?
- [ ] SQL 문법 오류는 HTTP 400인가요?

답변 템플릿:

```text
현재 API 계약은 POST /query 하나입니다. body 전체를 SQL 문자열로 읽고, SQL 엔진에 넘긴 뒤 JSON으로 응답하는 단순한 구조입니다.
```

```text
Content-Length는 body를 정확히 몇 byte 읽어야 하는지 알기 위해 필요합니다. 없으면 400 Bad Request로 처리합니다.
```

```text
SQL 문법 오류는 HTTP 요청 자체는 정상적으로 들어온 것이므로 현재 구현에서는 200 응답 안에 ok:false JSON으로 반환합니다. 반면 GET /query나 잘못된 path는 HTTP 계약 위반이라 405나 404를 반환합니다.
```

API/HTTP 백업 완료 기준:

- [ ] HTTP parser가 어디까지 지원하는지 설명할 수 있다.
- [ ] `POST /query` API 계약을 설명할 수 있다.
- [ ] HTTP 오류와 SQL 오류를 구분할 수 있다.

## 8-3. 동시성 백업 역할

담당 파일:

- [ ] `server/thread_pool.h`
- [ ] `server/thread_pool.c`
- [ ] `server/server.c`
- [ ] `server/api.c`
- [ ] `server_main.c`

담당 개념:

- [ ] worker thread
- [ ] bounded queue
- [ ] circular queue
- [ ] `head`, `tail`, `size`
- [ ] queue mutex
- [ ] condition variable
- [ ] shutdown flag
- [ ] DB mutex
- [ ] queue full 503

답변해야 할 질문:

- [ ] 왜 thread pool을 썼나요?
- [ ] worker 수는 몇 개인가요?
- [ ] queue 크기는 몇 개인가요?
- [ ] queue가 가득 차면 어떻게 되나요?
- [ ] condition variable은 왜 필요한가요?
- [ ] DB mutex는 왜 필요한가요?
- [ ] 이 구조는 완전히 병렬인가요?

답변 템플릿:

```text
요청마다 thread를 만들면 생성/해제 비용이 크고 요청 폭주 시 thread 수가 무제한으로 늘 수 있습니다. 그래서 worker를 미리 만들어두고 bounded queue에 들어온 connection을 처리하게 했습니다.
```

```text
queue가 비어 있으면 worker는 condition variable로 잠들고, 새 job이 들어오면 submit이 signal로 worker를 깨웁니다.
```

```text
네트워크 요청 파싱과 응답 처리는 여러 worker에서 병렬로 처리됩니다. 다만 shared Table과 B+Tree는 DB mutex로 보호되므로 SQL 실행 구간은 직렬화됩니다.
```

동시성 백업 완료 기준:

- [ ] thread pool 구조를 설명할 수 있다.
- [ ] queue full 시 503 흐름을 설명할 수 있다.
- [ ] DB mutex와 queue mutex의 차이를 설명할 수 있다.
- [ ] 현재 구조의 병렬성과 한계를 정확히 말할 수 있다.

## 8-4. DB 백업 역할

담당 파일:

- [ ] `sql_processor/sql.h`
- [ ] `sql_processor/sql.c`
- [ ] `sql_processor/table.h`
- [ ] `sql_processor/table.c`
- [ ] `sql_processor/bptree.h`
- [ ] `sql_processor/bptree.c`
- [ ] `server/api.c`

담당 개념:

- [ ] `sql_execute`
- [ ] `SQLResult`
- [ ] `Table`
- [ ] `Record`
- [ ] `next_id`
- [ ] `rows`
- [ ] `pk_index`
- [ ] B+Tree id index
- [ ] name/age linear scan

답변해야 할 질문:

- [ ] 지원하는 SQL은 무엇인가요?
- [ ] `id`는 사용자가 직접 넣나요?
- [ ] 왜 `id` 조회는 B+Tree를 쓰나요?
- [ ] 왜 `name`, `age`는 선형 탐색인가요?
- [ ] B+Tree에는 무엇이 저장되나요?
- [ ] SELECT 결과는 어떤 구조로 API에 전달되나요?

답변 템플릿:

```text
지원 SQL은 INSERT INTO users VALUES ('name', age)와 SELECT * FROM users, 그리고 id, name, age 조건 조회입니다. table은 users 하나이고 컬럼은 id, name, age입니다.
```

```text
id는 auto increment로 생성되고, B+Tree에는 id를 key로 Record pointer를 value로 저장합니다. 그래서 id 조건 조회는 B+Tree index를 사용합니다.
```

```text
name과 age에는 별도 secondary index가 없어서 rows 배열을 처음부터 순회하는 선형 탐색으로 처리합니다.
```

DB 백업 완료 기준:

- [ ] 지원 SQL과 제한 사항을 설명할 수 있다.
- [ ] `Table` 구조체를 설명할 수 있다.
- [ ] B+Tree index와 선형 탐색의 차이를 설명할 수 있다.
- [ ] `SQLResult`가 JSON으로 바뀌는 흐름을 설명할 수 있다.

## 8-5. 테스트 백업 역할

담당 파일:

- [ ] `sql_processor/unit_test.c`
- [ ] `scripts/smoke_test.sh`
- [ ] `Makefile`
- [ ] `sql_processor/Makefile`

담당 개념:

- [ ] clean build
- [ ] SQL unit test
- [ ] smoke test
- [ ] curl 기능 테스트
- [ ] 동시 INSERT 테스트
- [ ] 오류 SQL 테스트

답변해야 할 질문:

- [ ] 어떤 테스트를 실행했나요?
- [ ] unit test는 무엇을 검증하나요?
- [ ] smoke test는 무엇을 검증하나요?
- [ ] 동시성 테스트는 무엇을 확인하나요?
- [ ] 오류 SQL은 어떻게 처리되나요?
- [ ] 테스트 한계는 무엇인가요?

답변 템플릿:

```text
SQL 엔진은 unit_test로 B+Tree, Table, SQL insert/select, 조건 조회, 오류 처리를 검증했습니다.
```

```text
API 서버는 smoke_test.sh로 INSERT와 SELECT가 HTTP API를 통해 정상 연결되는지 확인했습니다.
```

```text
동시 INSERT는 xargs -P8로 여러 요청을 동시에 보내고, inserted_id가 중복되지 않고 서버가 유지되는지 확인했습니다.
```

테스트 백업 완료 기준:

- [ ] unit test와 smoke test의 차이를 설명할 수 있다.
- [ ] 동시 INSERT 테스트의 검증 기준을 설명할 수 있다.
- [ ] 오류 SQL이 crash가 아니라 JSON 오류로 반환된다는 점을 설명할 수 있다.

## 8-6. 자주 나올 질문 모음

질문: 왜 C로 구현했나요?

```text
과제 요구사항이 C 언어였고, socket, thread, mutex 같은 시스템 프로그래밍 요소를 직접 다루는 것이 이번 주 핵심이었습니다.
```

질문: 왜 HTTP parser를 직접 만들었나요?

```text
과제 범위가 단일 endpoint인 POST /query라서, Content-Length 기반의 최소 HTTP parser로 충분했습니다. production-grade HTTP 서버가 아니라 DBMS API 연결 구조를 보여주는 것이 목적입니다.
```

질문: 왜 JSON 요청이 아니라 SQL 문자열을 body로 받나요?

```text
기존 SQL 엔진의 입력이 문자열이기 때문에 API adapter를 단순하게 유지하기 위해 body 전체를 SQL 문자열로 사용했습니다.
```

질문: DB mutex 때문에 성능이 떨어지지 않나요?

```text
맞습니다. 전역 DB mutex는 안정성을 우선한 설계라 DB 실행 구간은 직렬화됩니다. 개선한다면 SELECT는 read lock, INSERT는 write lock을 쓰는 rwlock 구조로 바꿀 수 있습니다.
```

질문: B+Tree를 쓰는 효과는 무엇인가요?

```text
id equality 조회는 전체 row를 순회하지 않고 index를 따라가 찾을 수 있고, range query는 leaf chain을 따라가며 key 순서로 결과를 모을 수 있습니다.
```

질문: 데이터는 파일에 저장되나요?

```text
아직은 in-memory only입니다. 서버 프로세스가 살아 있는 동안만 유지되고, 재시작하면 사라집니다. persistence는 개선 방향입니다.
```

질문: SQL injection 문제는 없나요?

```text
이 프로젝트는 일반 웹 서비스처럼 사용자 입력을 조합해 SQL을 만드는 구조가 아니라, 클라이언트가 SQL 자체를 보내는 DBMS API입니다. 다만 인증과 권한 제어는 아직 없어서 production 환경에서는 추가가 필요합니다.
```

질문: queue가 가득 차면 요청은 어떻게 되나요?

```text
thread_pool_submit이 실패하고 server_run에서 503 Service Unavailable과 queue_full JSON을 반환합니다.
```

## 8-7. 팀 리허설 방법

1차 리허설:

- [ ] 대표 발표자가 4분 발표를 한다.
- [ ] 팀원은 중간에 끊지 않고 질문만 메모한다.
- [ ] 끝난 뒤 빠진 키워드를 체크한다.

2차 리허설:

- [ ] 대표 발표자가 데모까지 포함해 발표한다.
- [ ] 각 백업 담당자가 자기 영역 질문에 답한다.
- [ ] 답변이 길면 20초 버전으로 줄인다.

3차 리허설:

- [ ] 실제 발표처럼 4분 타이머를 켠다.
- [ ] Q&A 1분을 추가로 진행한다.
- [ ] 대답 못 한 질문은 담당 파일을 다시 읽는다.

## 8-8. 팀원별 체크표

대표 발표자:

- [ ] 4분 발표 가능
- [ ] 데모 명령 숙지
- [ ] 전체 요청 흐름 설명 가능

API/HTTP 백업:

- [ ] `POST /query` 설명 가능
- [ ] HTTP status code 설명 가능
- [ ] SQL 오류와 HTTP 오류 구분 가능

동시성 백업:

- [ ] thread pool 설명 가능
- [ ] bounded queue 설명 가능
- [ ] DB mutex 설명 가능

DB 백업:

- [ ] `sql_execute` 설명 가능
- [ ] Table/B+Tree 설명 가능
- [ ] id index와 name/age scan 차이 설명 가능

테스트 백업:

- [ ] unit test 설명 가능
- [ ] smoke test 설명 가능
- [ ] concurrency test 설명 가능

## 완료 기준

- [ ] 대표 발표자가 전체 발표를 4분 안에 끝냈다.
- [ ] 팀원별 백업 영역이 정해졌다.
- [ ] 각 담당자가 자기 영역 질문 3개 이상에 답했다.
- [ ] 답변이 너무 긴 질문은 20초 답변으로 줄였다.
- [ ] 마지막으로 전체 팀이 핵심 질문 5개에 답했다.

## 마지막 핵심 질문 5개

- [ ] 이 프로젝트를 한 문장으로 설명하면?
- [ ] 요청 하나가 들어오면 코드에서 어떤 순서로 처리되는가?
- [ ] 왜 thread pool을 썼는가?
- [ ] 왜 DB mutex가 필요한가?
- [ ] `WHERE id >= 1`은 왜 B+Tree를 쓰고, `name/age`는 왜 선형 탐색인가?
