# 메모리 기반 B+Tree SQL 처리기

이 문서는 메모리 기반 `users` 테이블과 B+Tree 인덱스로 동작하는 미니 SQL 처리기의 구조를 설명합니다.

이번 프로젝트의 핵심은 SQL 문장을 실행하는 데서 끝나는 것이 아니라, 팀원 모두가 코드의 흐름을 직접 설명할 수 있을 정도로 이해하는 것입니다.

## 어떤 프로젝트인가

이 프로젝트는 C 언어로 만든 메모리 기반 미니 SQL 처리기입니다.

사용자는 REPL에 SQL 문장을 입력하고, 프로그램은 이를 해석해 `users` 테이블에 저장하거나 조회합니다.

```text
입력(SQL) -> 파싱 -> 실행 -> 테이블 저장 / 인덱스 검색 -> 결과 출력
```

현재 구현은 범용 DBMS가 아니라 아래 목표에 집중한 학습용 MVP입니다.

- SQL 처리 흐름 이해
- B+Tree 인덱스 동작 이해
- split과 leaf link를 직접 설명할 수 있는 수준까지 소스코드 이해
- 인덱스 검색과 선형 탐색의 차이 확인

## 현재 지원 범위

- `INSERT INTO users VALUES ('Alice', 20);`
- `SELECT * FROM users;`
- `SELECT * FROM users WHERE id = 1;`
- `SELECT * FROM users WHERE id >= 10;`
- `SELECT * FROM users WHERE name = 'Alice';`
- `SELECT * FROM users WHERE age > 20;`
- `id` 자동 증가
- `id` 기준 B+Tree 인덱스 검색
- `name`, `age` 기준 선형 탐색

즉, 이 프로젝트는 작은 SQL 처리기 위에 B+Tree 인덱스를 붙여 DB 내부 동작을 설명 가능한 수준까지 재구성한 학습 프로젝트입니다.

## 프로젝트 구성

핵심 흐름은 다음과 같습니다.

```text
사용자 SQL 입력 -> 파싱 -> 실행 -> 테이블 저장 / 인덱스 검색 -> 결과 출력
```

구현 대상은 크게 세 부분입니다.

- SQL 처리기
- 테이블 저장소
- B+Tree 인덱스

## 코드 구조

주요 파일은 다음과 같습니다.

- `main.c`: REPL 실행 진입점
- `sql.c`: SQL 파싱과 실행 로직
- `table.c`: `users` 테이블 저장, 검색, 출력
- `bptree.c`: B+Tree 구현
- `unit_test.c`: 단위 테스트
- `perf_test.c`: `id` 1000개 검색 성능 비교
- `condition_perf_test.c`: 조건 검색 성능 비교
- `Makefile`: 빌드 스크립트
- `docs/diagrams/*.mmd`: Mermaid 다이어그램

## 자료 구조

### Record

`users` 테이블의 한 행을 표현합니다.

```c
typedef struct Record {
    int id;
    char name[RECORD_NAME_SIZE];
    int age;
} Record;
```

- `id`: 자동 증가 PK
- `name`: 사용자 이름
- `age`: 정수 나이

### Table

테이블 본체와 인덱스를 함께 관리합니다.

```c
typedef struct Table {
    int next_id;
    Record **rows;
    size_t size;
    size_t capacity;
    BPTree *pk_index;
} Table;
```

- `rows`: 실제 저장된 row 배열
- `pk_index`: `id` 기준 B+Tree
- `next_id`: INSERT 시 자동 증가하는 값

### BPTreeNode

B+Tree 노드가 key와 자식 또는 값을 함께 가집니다.

```c
typedef struct BPTreeNode {
    int is_leaf;
    int num_keys;
    int keys[BPTREE_MAX_KEYS];
    void *values[BPTREE_MAX_KEYS];
    struct BPTreeNode *children[BPTREE_ORDER];
    struct BPTreeNode *parent;
    struct BPTreeNode *next;
} BPTreeNode;
```

- leaf 노드: 실제 `(id, Record *)`를 저장
- internal 노드: 탐색 경로를 안내
- `next`: leaf 노드 사이의 연결

### SQLResult

SQL 실행 결과를 호출자에게 돌려주는 구조체입니다.

- 성공/실패 상태
- INSERT된 id
- SELECT 결과 row 목록
- SQL 오류 메시지

## 테스트

### 단위 테스트

```bash
make
./unit_test
```

검증 항목:

- 빈 트리 검색
- 단일 key insert/search
- duplicate key 거부
- leaf split 이후 검색 유지
- internal split 이후 검색 유지
- leaf next 링크 유지
- auto increment
- `WHERE id`, `WHERE age` 조건 검색
- SQL 실행 결과 구조

### 성능 테스트

```bash
./perf_test
./condition_perf_test
```

이 테스트는 B+Tree 검색과 선형 검색의 차이를 비교할 때 사용합니다.

## 실행 예시

### 기본 실행

```bash
make
./main
```

### REPL 입력 예시

```sql
INSERT INTO users VALUES ('Alice', 20);
INSERT INTO users VALUES ('Bob', 30);
SELECT * FROM users;
SELECT * FROM users WHERE id = 1;
SELECT * FROM users WHERE id >= 2;
SELECT * FROM users WHERE name = 'Bob';
SELECT * FROM users WHERE age > 20;
QUIT
```

