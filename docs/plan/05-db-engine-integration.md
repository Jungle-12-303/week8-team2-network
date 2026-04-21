# 05. DB Engine Integration

## 코드 주석 원칙

DB 엔진 연결 코드를 작성할 때는 `sql_execute()`, `SQLResult`, `Table`, `Record`, B+Tree 인덱스의 관계를 한국어 주석으로 친절하게 설명합니다. 특히 `SQLResult.records`는 포인터 배열이고 실제 row는 `Table`이 소유한다는 점을 응답 변환 코드 근처에 남깁니다.

이 문서는 기존 `sql_processor` 엔진을 API 서버에 연결하는 방식을 설명합니다.

## 기존 엔진 요약

기존 엔진은 다음 구조를 가지고 있습니다.

- `Table`: 메모리 기반 `users` 테이블입니다.
- `Record`: `id`, `name`, `age`를 가진 row입니다.
- `BPTree`: `id` 기반 primary key index입니다.
- `sql_execute(Table *table, const char *input)`: SQL 문자열을 파싱하고 실행합니다.
- `SQLResult`: 실행 결과, row 목록, 오류 메시지를 담습니다.

## 서버에서 사용할 진입점

API 서버는 SQL을 직접 파싱하지 않고 기존 함수를 호출합니다.

```c
SQLResult result = sql_execute(shared_table, sql_body);
```

이 방식의 장점:

- 기존 단위 테스트가 검증한 SQL 처리 로직을 그대로 재사용합니다.
- API 서버는 네트워크, HTTP, 동시성에 집중할 수 있습니다.
- 발표 때 “DB 엔진”과 “API 서버”의 책임 분리를 설명하기 쉽습니다.

## 서버 시작과 종료

서버 시작 시:

```c
Table *shared_table = table_create();
```

서버 종료 시:

```c
table_destroy(shared_table);
```

`shared_table`은 모든 worker가 공유합니다. 따라서 [04-thread-pool-and-concurrency.md](04-thread-pool-and-concurrency.md)의 DB mutex 정책을 반드시 적용합니다.

## SQLResult 변환 정책

서버는 `SQLResult`를 HTTP JSON body로 변환합니다.

- `SQL_STATUS_OK` + `SQL_ACTION_INSERT`: inserted id와 row count 반환.
- `SQL_STATUS_OK` + `SQL_ACTION_SELECT_ROWS`: rows 배열 반환.
- `SQL_STATUS_NOT_FOUND`: 성공 응답으로 보고 빈 rows 배열 반환.
- `SQL_STATUS_SYNTAX_ERROR`: `ok: false`와 SQL 오류 메시지 반환.
- `SQL_STATUS_QUERY_ERROR`: `ok: false`와 query 오류 메시지 반환.
- `SQL_STATUS_ERROR`: 서버 내부 오류로 처리.
- `SQL_STATUS_EXIT`: API에서는 종료 명령으로 사용하지 않습니다. `EXIT`, `QUIT` 요청은 오류 또는 no-op로 처리합니다.

## 메모리 관리

- `sql_execute()`가 반환한 `SQLResult`는 사용 후 반드시 `sql_result_destroy()`로 정리합니다.
- SELECT 결과의 `records` 배열은 `SQLResult`가 소유하지만, 각 `Record` 자체는 `Table`이 소유합니다.
- 응답 문자열을 완성한 뒤 `sql_result_destroy()`를 호출합니다.
- 응답 쓰기는 이미 완성된 문자열을 사용하므로 DB lock 밖에서 수행해도 됩니다.

## 기존 코드 보존 원칙

- 가능하면 `sql.c`, `table.c`, `bptree.c`의 동작은 바꾸지 않습니다.
- 서버에 필요한 보조 변환은 API 계층에서 처리합니다.
- 기존 `main.c` REPL은 디버깅용으로 유지할 수 있습니다.
- 기존 `unit_test`, `perf_test`, `condition_perf_test`는 계속 빌드되고 실행되어야 합니다.
