# 5단계. DB 엔진 연결 이해하기

## 목표

기존 SQL 처리기와 B+Tree 인덱스가 API 서버와 어떻게 연결되는지 이해합니다. 발표에서는 “기존 DB 엔진 재사용”과 “id 조회는 B+Tree, name/age는 선형 탐색”을 명확히 설명해야 합니다.

## 예상 소요 시간

- 50~70분

## 읽을 파일 순서

- [ ] `sql_processor/sql.h`
- [ ] `sql_processor/sql.c`
- [ ] `sql_processor/table.h`
- [ ] `sql_processor/table.c`
- [ ] `sql_processor/bptree.h`
- [ ] `sql_processor/bptree.c`
- [ ] `server/api.c`

## 5-1. API와 DB 엔진 연결점 찾기

파일:

- [ ] `server/api.c`

확인할 코드:

- [ ] `api_handle_query`
- [ ] `sql_execute(table, sql)`
- [ ] `SQLResult sql_result`
- [ ] `api_build_success_response`
- [ ] `api_build_error_response`

연결 흐름:

```text
HTTP body의 SQL 문자열
-> api_handle_query
-> sql_execute(Table *table, const char *input)
-> SQLResult
-> JSON response body
```

체크 질문:

- [ ] API 서버가 SQL parser를 새로 구현했는가?
- [ ] SQL 실행의 진입점 함수는 무엇인가?
- [ ] SQL 실행 결과는 어떤 구조체로 돌아오는가?

답변 힌트:

```text
API 서버는 SQL을 직접 파싱하지 않고 기존 sql_processor의 sql_execute 함수를 호출합니다.
```

## 5-2. `SQLResult` 구조 이해

파일:

- [ ] `sql_processor/sql.h`

확인할 타입:

- [ ] `SQLStatus`
- [ ] `SQLAction`
- [ ] `SQLResult`

`SQLStatus` 의미:

- [ ] `SQL_STATUS_OK`: SQL 실행 성공
- [ ] `SQL_STATUS_NOT_FOUND`: 조회 결과 없음
- [ ] `SQL_STATUS_SYNTAX_ERROR`: SQL 문법 오류
- [ ] `SQL_STATUS_QUERY_ERROR`: 컬럼 오류 같은 쿼리 오류
- [ ] `SQL_STATUS_EXIT`: EXIT 또는 QUIT
- [ ] `SQL_STATUS_ERROR`: 내부 오류

`SQLAction` 의미:

- [ ] `SQL_ACTION_NONE`
- [ ] `SQL_ACTION_INSERT`
- [ ] `SQL_ACTION_SELECT_ROWS`

`SQLResult` 필드:

- [ ] `status`
- [ ] `action`
- [ ] `record`
- [ ] `records`
- [ ] `inserted_id`
- [ ] `row_count`
- [ ] `error_code`
- [ ] `sql_state`
- [ ] `error_message`

체크 질문:

- [ ] INSERT 결과에서 중요한 필드는 무엇인가?
- [ ] SELECT 결과에서 중요한 필드는 무엇인가?
- [ ] 오류 결과에서 중요한 필드는 무엇인가?

답변 힌트:

```text
INSERT는 inserted_id와 row_count를 보고, SELECT는 records와 row_count를 보고, 오류는 status, error_code, sql_state, error_message를 봅니다.
```

## 5-3. 지원 SQL 범위 정리

지원되는 SQL:

- [ ] `INSERT INTO users VALUES ('Alice', 20);`
- [ ] `SELECT * FROM users;`
- [ ] `SELECT * FROM users WHERE id = 1;`
- [ ] `SELECT * FROM users WHERE id >= 10;`
- [ ] `SELECT * FROM users WHERE name = 'Alice';`
- [ ] `SELECT * FROM users WHERE age > 20;`

제한 사항:

- [ ] table은 `users`만 지원한다.
- [ ] SELECT list는 `*`만 실질적으로 지원한다.
- [ ] 컬럼은 `id`, `name`, `age`만 지원한다.
- [ ] INSERT는 name과 age만 받는다.
- [ ] id는 자동 증가한다.
- [ ] name 조건은 `=`만 지원한다.
- [ ] age와 id는 `=`, `<`, `<=`, `>`, `>=`를 지원한다.
- [ ] JOIN, UPDATE, DELETE, ORDER BY, GROUP BY는 지원하지 않는다.

체크 질문:

- [ ] `SELECT name FROM users;`는 지원되는가?
- [ ] `SELECT * FROM users WHERE name > 'Alice';`는 지원되는가?
- [ ] `INSERT INTO users VALUES (1, 'Alice', 20);`는 지원되는가?

답변 힌트:

```text
현재 구현은 발표와 과제 범위에 맞춘 제한된 SQL 문법만 지원합니다.
```

## 5-4. `sql_execute` 흐름 이해

파일:

- [ ] `sql_processor/sql.c`

확인할 함수:

- [ ] `sql_execute`
- [ ] `sql_execute_insert`
- [ ] `sql_execute_select`
- [ ] `sql_set_syntax_error`
- [ ] `sql_set_unknown_column_error`

`sql_execute` 흐름:

```text
입력 공백 제거
-> EXIT/QUIT 검사
-> INSERT 문법 시도
-> SELECT 문법 시도
-> 실패 시 syntax error 생성
```

INSERT 흐름:

```text
INSERT
-> INTO
-> users
-> VALUES
-> '('
-> string name
-> ','
-> int age
-> ')'
-> optional semicolon
-> table_insert
```

SELECT 흐름:

```text
SELECT
-> *
-> FROM
-> users
-> optional WHERE
-> table 조회 함수 호출
```

체크 질문:

- [ ] SQL keyword는 대소문자를 구분하는가?
- [ ] 세미콜론은 필수인가?
- [ ] SQL 문법 오류는 어떤 status로 표현되는가?

답변 힌트:

```text
keyword는 case-insensitive로 처리하고, statement end에서는 optional semicolon과 trailing whitespace를 허용합니다.
```

## 5-5. `Table` 구조 이해

파일:

- [ ] `sql_processor/table.h`

확인할 구조체:

```c
typedef struct Table {
    int next_id;
    Record **rows;
    size_t size;
    size_t capacity;
    BPTree *pk_index;
} Table;
```

필드 의미:

- [ ] `next_id`: 다음 INSERT에 부여할 auto increment id
- [ ] `rows`: Record pointer 배열
- [ ] `size`: 현재 row 수
- [ ] `capacity`: rows 배열 용량
- [ ] `pk_index`: id 기반 B+Tree index

`Record` 필드:

- [ ] `id`
- [ ] `name`
- [ ] `age`

체크 질문:

- [ ] 실제 row 데이터는 어디에 저장되는가?
- [ ] B+Tree에는 Record 값 전체가 복사되는가, pointer가 들어가는가?
- [ ] `rows` 배열과 B+Tree index는 어떤 관계인가?

답변 힌트:

```text
Record는 heap에 할당되고 rows 배열은 Record pointer를 보관합니다. B+Tree도 id를 key로 하고 Record pointer를 value로 보관합니다.
```

## 5-6. INSERT와 B+Tree 연결 보기

파일:

- [ ] `sql_processor/table.c`

확인할 함수:

- [ ] `table_insert`
- [ ] `table_ensure_capacity`
- [ ] `bptree_insert`

INSERT 흐름:

```text
table_ensure_capacity
-> Record calloc
-> record->id = table->next_id++
-> name 복사
-> age 저장
-> rows[table->size] = record
-> bptree_insert(pk_index, record->id, record)
-> table->size++
```

체크 질문:

- [ ] id는 사용자가 입력하는가, 자동으로 만들어지는가?
- [ ] rows 배열에 넣은 뒤 B+Tree insert가 실패하면 어떤 문제가 있을 수 있는가?
- [ ] B+Tree insert가 성공하면 어떤 조회가 빨라지는가?

답변 힌트:

```text
id는 자동 증가하고, B+Tree에는 id와 Record pointer가 들어가므로 id 기반 조회가 index를 사용할 수 있습니다.
```

## 5-7. id 조회가 B+Tree를 쓰는 흐름 보기

파일:

- [ ] `sql_processor/table.c`

확인할 함수:

- [ ] `table_find_by_id`
- [ ] `table_find_by_id_condition`
- [ ] `table_find_id_leaf`
- [ ] `table_find_leftmost_leaf`
- [ ] `table_collect_leaf_chain`

id equality 조회:

```text
WHERE id = 1
-> table_find_by_id_condition
-> table_find_by_id
-> bptree_search
```

id range 조회:

```text
WHERE id >= 3
-> table_find_id_leaf
-> leaf 안에서 시작 index 찾기
-> leaf linked list를 따라가며 records 수집
```

id less-than 조회:

```text
WHERE id < 3
-> leftmost leaf부터 시작
-> 조건이 깨질 때까지 leaf를 순회
```

체크 질문:

- [ ] `WHERE id = 1`은 어떤 B+Tree 함수를 쓰는가?
- [ ] `WHERE id >= 1`은 왜 leaf chain이 유리한가?
- [ ] B+Tree leaf에 `next` pointer가 있는 이유는 무엇인가?

답변 힌트:

```text
B+Tree leaf들은 key 순서로 연결되어 있어서 range scan을 할 때 시작 leaf를 찾은 뒤 다음 leaf로 이어서 읽을 수 있습니다.
```

## 5-8. name과 age 조회가 선형 탐색인 이유

파일:

- [ ] `sql_processor/table.c`

확인할 함수:

- [ ] `table_find_by_name_matches`
- [ ] `table_find_by_age_condition`
- [ ] `table_find_by_name`
- [ ] `table_find_by_age`

name 조회 흐름:

```text
for index in 0..table->size-1
-> strcmp(table->rows[index]->name, name)
-> match면 result 배열에 append
```

age 조회 흐름:

```text
for index in 0..table->size-1
-> table_compare_int(row->age, comparison, age)
-> match면 result 배열에 append
```

체크 질문:

- [ ] name용 B+Tree가 있는가?
- [ ] age용 B+Tree가 있는가?
- [ ] row가 많아질수록 name/age 조회 성능은 어떻게 되는가?

답변 힌트:

```text
현재 index는 id에만 있으므로 name과 age는 rows 배열을 처음부터 끝까지 훑는 선형 탐색입니다.
```

## 5-9. `SQLResult`를 JSON으로 바꾸는 흐름

파일:

- [ ] `server/api.c`
- [ ] `server/json_util.c`

성공 INSERT JSON:

```json
{"ok":true,"action":"insert","inserted_id":1,"row_count":1}
```

성공 SELECT JSON:

```json
{"ok":true,"action":"select","row_count":1,"rows":[{"id":1,"name":"Alice","age":20}]}
```

SQL 오류 JSON:

```json
{"ok":false,"status":"syntax_error","error_code":1064,"sql_state":"42000","message":"..."}
```

확인할 함수:

- [ ] `api_build_success_response`
- [ ] `api_build_error_response`
- [ ] `api_append_record`
- [ ] `json_buffer_append_json_string`

체크 질문:

- [ ] 문자열을 JSON에 넣을 때 escape 처리가 필요한 이유는 무엇인가?
- [ ] SELECT 결과가 여러 row일 때 JSON 배열은 어디서 만들어지는가?
- [ ] SQL error response와 HTTP error response body는 같은 함수에서 만들어지는가?

답변 힌트:

```text
SQLResult 기반 응답은 api.c에서 만들고, HTTP parser 단계의 오류 응답은 server.c의 server_build_error_body에서 만듭니다.
```

## 5-10. 현재 DB 엔진 한계와 개선 방향

현재 한계:

- [ ] in-memory only
- [ ] 단일 `users` table
- [ ] 제한된 SQL grammar
- [ ] id index만 있음
- [ ] transaction 없음
- [ ] persistence 없음
- [ ] global DB mutex

개선 방향:

- [ ] file persistence 추가
- [ ] multi-table catalog 추가
- [ ] CREATE TABLE 지원
- [ ] UPDATE, DELETE 지원
- [ ] name/age secondary index 추가
- [ ] SELECT는 read lock, INSERT는 write lock을 쓰는 rwlock 구조
- [ ] SQL parser 확장

체크 질문:

- [ ] 포트폴리오에서 이 프로젝트의 강점은 무엇인가?
- [ ] 한계를 개선 계획으로 바꾸어 말하려면 어떻게 해야 하는가?

답변 힌트:

```text
강점은 SQL 엔진, B+Tree index, HTTP API 서버, thread pool을 C로 직접 연결했다는 점입니다. 한계는 향후 개선 방향으로 제시하면 됩니다.
```

## 발표용 핵심 문장

```text
기존 SQL 처리기는 sql_execute(Table *table, const char *input) 하나로 감싸져 있어서, API 서버는 SQL parser를 새로 만들지 않고 이 함수를 호출하는 방식으로 재사용했습니다.
```

```text
id는 primary key처럼 증가하는 값이고 B+Tree에 key-value 형태로 저장됩니다. 그래서 WHERE id 조건은 B+Tree index를 따라가며 찾습니다.
```

```text
반면 name, age에는 별도 index가 없어서 row 배열을 처음부터 순회합니다.
```

## 완료 기준

- [ ] `sql_execute`의 입력과 출력을 설명할 수 있다.
- [ ] `SQLResult` 필드를 설명할 수 있다.
- [ ] 지원 SQL과 미지원 SQL을 구분할 수 있다.
- [ ] `Table` 구조체의 필드를 설명할 수 있다.
- [ ] INSERT 시 rows 배열과 B+Tree가 어떻게 함께 갱신되는지 설명할 수 있다.
- [ ] `WHERE id` 조회가 B+Tree를 쓰는 이유를 설명할 수 있다.
- [ ] `WHERE name/age` 조회가 선형 탐색인 이유를 설명할 수 있다.
- [ ] DB 엔진의 현재 한계와 개선 방향을 말할 수 있다.
