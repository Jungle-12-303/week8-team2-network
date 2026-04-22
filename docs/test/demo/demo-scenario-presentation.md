# 발표용 데모 시나리오

이 문서는 `make demo`를 발표장에서 짧고 안정적으로 보여주기 위한 안내서입니다.
핵심은 3단계입니다.

1. 내부 DB 엔진과 API 서버가 연결되는지 보여준다.
2. 짧은 동시성 스모크 테스트로 `SELECT`와 `INSERT`의 read lock / write lock 동작을 확인한다. 이 단계는 1단계에서 띄운 서버를 그대로 재사용하며, 전체 스캔 대신 대표 row 조회로 빠르게 끝낸다.
3. API 서버의 thread pool, job queue, 503 제어를 보여준다.

## 실행 방법

기본 실행은 아래 명령 하나면 됩니다.

```bash
make demo
```

긴 동시성 스트레스는 발표 기본 흐름에서 제외하고, 필요할 때만 옵션으로 실행합니다.

```bash
DEMO_INCLUDE_LONG_CONCURRENCY=1 make demo
```

Windows에서는 다음 배치파일도 사용할 수 있습니다.

```bat
scripts\demo\demo_scenario.bat
```

macOS와 Linux에서는 셸 스크립트로도 실행할 수 있습니다.

```bash
sh scripts/demo/demo_scenario.sh
```

## 기대 결과

### 1단계. DB 엔진과 API 서버 연결

화면에 다음과 비슷한 흐름이 보여야 합니다.

```text
[1/3] Internal DB engine and API server link
SQL> INSERT INTO users VALUES ('Alice', 20);
SQL> SELECT * FROM users WHERE id = 1;
```

기대값은 다음입니다.

- `INSERT` 응답에 `ok:true`, `action:"insert"`, `inserted_id:1`, `row_count:1`
- `SELECT` 응답에 `ok:true`, `action:"select"`, `row_count:1`, `rows` 안에 `Alice`

### 2단계. 짧은 동시성 스모크 테스트

이 단계는 발표용으로 짧게 유지한 부분입니다.
1단계에서 이미 띄운 서버를 그대로 재사용하므로, 보통 3초 안팎으로 끝납니다.

화면에는 다음과 같은 제목이 보여야 합니다.

```text
[2/3] RW lock concurrency smoke test
```

그리고 마지막에는 아래 메시지가 보여야 합니다.

```text
rwlock quick demo test passed.
```

이 단계가 보여주는 핵심은 이렇습니다.

- `SELECT`는 read lock을 잡는다.
- `INSERT`는 write lock을 잡는다.
- 읽기 요청은 동시에 가능하다.
- 쓰기 요청은 배타적으로 처리된다.
- 이 시나리오는 `SELECT *` 전체 스캔이 아니라 `SELECT * WHERE id = 1`처럼 가벼운 읽기로 짧게 확인한다.

즉, 긴 스트레스가 아니라도 “읽기와 쓰기가 동시에 들어와도 서버가 안전하게 동작한다”는 점을 짧게 확인할 수 있습니다.

### 3단계. API 서버 아키텍처

화면에는 다음이 보여야 합니다.

```text
[3/3] API server architecture
요약: 20개 OK | 5개 503 | 0개 기타
통과
```

이 단계에서 확인할 포인트는 다음입니다.

- worker thread가 요청을 처리한다.
- job queue가 요청을 버퍼링한다.
- queue가 가득 차면 503으로 거절한다.

## 발표에서 말하면 좋은 한 줄

- 1단계는 “API와 내부 DB 엔진이 실제로 연결됐다”를 보여준다.
- 2단계는 “긴 스트레스 대신, 짧은 동시성 스모크 테스트로 read/write lock을 확인한다”를 보여준다.
- 3단계는 “thread pool과 queue로 부하를 제어한다”를 보여준다.

## 왜 이 구성이 좋은가

- 긴 스트레스 테스트를 기본 흐름에서 빼서 발표가 늘어지지 않는다.
- 대신 동시성의 핵심 개념은 그대로 보여줄 수 있다.
- 실패 확률이 낮고, 출력도 명확해서 데모용으로 적합하다.
