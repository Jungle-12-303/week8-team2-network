# Unit Tests

이 문서는 `sql_processor`의 단위 테스트를 사람이 직접 실행하고 확인하는 방법을 설명합니다.

## 무엇을 테스트하나

`sql_processor/unit_test.c`는 다음을 확인합니다.

- 빈 B+Tree 검색
- 단일 key insert/search
- duplicate key 거부
- leaf split 이후 검색 유지
- internal split 이후 검색 유지
- leaf next 링크 유지
- table auto increment
- `id` 기반 검색
- `name`, `age` 기반 선형 검색
- 조건 검색 결과 수집
- SQL 실행 결과 구조 검증

즉, 이 테스트는 HTTP 계층이 아니라 `B+Tree`, `Table`, `SQLResult`의 핵심 동작을 검증합니다.

## 수동 실행 방법

프로젝트 루트에서 아래 순서로 실행합니다.

```bash
cd sql_processor
make
./unit_test
```

프로젝트 루트에서 바로 한 번에 하려면 이렇게도 할 수 있습니다.

```bash
cd sql_processor; make; ./unit_test
```

PowerShell에서 현재 폴더가 이미 `sql_processor`라면 아래만 실행해도 됩니다.

```bash
make
./unit_test
```

## 기대 결과

정상적으로 통과하면 아래처럼 보입니다.

```text
All unit tests passed.
```

## 실패하면 무엇을 보면 되나

- `assert`가 실패한 줄
- `bptree.c`의 split/search 로직
- `table.c`의 insert/search 로직
- `sql.c`의 파싱 또는 조건 검색 로직

## 수동 확인 포인트

이 테스트는 별도의 HTTP 호출이 없습니다.
따라서 수동 확인은 다음만 보면 됩니다.

- 빌드가 성공하는가
- 실행이 끝까지 끝나는가
- 마지막에 `All unit tests passed.`가 출력되는가

## 실패했을 때 바로 볼 것

- `make`가 실패하면 컴파일 에러 메시지
- `./unit_test`가 중간에 멈추면 `assert`가 실패한 테스트 이름
- `bptree.c`, `table.c`, `sql.c` 중 어느 계층에서 실패했는지
