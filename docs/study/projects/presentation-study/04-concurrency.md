# 4단계. 동시성 구조 이해하기

## 목표

thread pool, bounded queue, mutex, condition variable, DB mutex를 발표와 Q&A 수준으로 이해합니다.

## 예상 소요 시간

- 50~60분

## 읽을 파일

- [ ] `server/thread_pool.h`
- [ ] `server/thread_pool.c`
- [ ] `server/server.c`
- [ ] `server/api.c`
- [ ] `server_main.c`

## 핵심 질문

- [ ] 왜 요청마다 새 thread를 만들지 않았는가?
- [ ] 왜 bounded queue가 필요한가?
- [ ] worker는 언제 잠들고 언제 깨어나는가?
- [ ] DB mutex는 왜 필요한가?
- [ ] 이 서버는 어느 부분이 병렬이고 어느 부분이 직렬인가?

## 4-1. thread pool이 필요한 이유 정리

먼저 개념을 말로 정리합니다.

```text
thread pool은 worker thread를 미리 만들어두고, 요청이 들어올 때마다 새 thread를 만들지 않고 queue에 작업을 넣어 기존 worker가 처리하게 하는 구조입니다.
```

장점:

- [ ] thread 생성/해제 비용을 줄인다.
- [ ] 동시에 실행되는 worker 수를 제한한다.
- [ ] 요청 폭주 시 queue 크기로 부하를 제어한다.
- [ ] 서버 구조가 예측 가능해진다.

체크 질문:

- [ ] 요청마다 thread를 만들면 어떤 문제가 생길 수 있는가?
- [ ] worker 수를 4개로 제한하면 어떤 장점이 있는가?
- [ ] worker 수가 많을수록 항상 좋은가?

답변 힌트:

```text
worker 수가 너무 많으면 context switching과 lock 경쟁이 늘어날 수 있습니다. 현재 프로젝트는 단순하고 안정적인 구조를 위해 worker 4개를 사용합니다.
```

## 4-2. ThreadPool 구조체 읽기

파일:

- [ ] `server/thread_pool.h`

확인할 필드:

- [ ] `pthread_t *workers`
- [ ] `size_t worker_count`
- [ ] `ThreadPoolJob *jobs`
- [ ] `size_t queue_capacity`
- [ ] `size_t head`
- [ ] `size_t tail`
- [ ] `size_t size`
- [ ] `int stop_requested`
- [ ] `pthread_mutex_t mutex`
- [ ] `pthread_cond_t cond`
- [ ] `ThreadPoolJobHandler handler`
- [ ] `void *context`

각 필드 설명:

- [ ] `workers`: 미리 생성한 worker thread 배열
- [ ] `jobs`: client fd를 담는 job queue
- [ ] `head`: 다음에 꺼낼 위치
- [ ] `tail`: 다음에 넣을 위치
- [ ] `size`: 현재 queue에 들어 있는 job 수
- [ ] `stop_requested`: shutdown 요청 여부
- [ ] `mutex`: queue 상태 보호
- [ ] `cond`: worker를 깨우는 condition variable
- [ ] `handler`: 실제 job 처리 함수
- [ ] `context`: handler에 넘길 서버 객체

체크 질문:

- [ ] thread pool mutex와 DB mutex는 같은 것인가?
- [ ] `jobs`에는 SQL 문자열이 들어가는가, client fd가 들어가는가?
- [ ] `context`는 왜 필요한가?

답변 힌트:

```text
thread pool queue에는 client fd만 들어갑니다. worker는 fd를 꺼낸 뒤 server_handle_client를 호출하고, context로 받은 Server에서 shared Table과 db_mutex를 사용합니다.
```

## 4-3. thread pool 초기화 보기

파일:

- [ ] `server/thread_pool.c`

확인할 함수:

- [ ] `thread_pool_init`

확인할 흐름:

- [ ] worker 배열 할당
- [ ] job queue 배열 할당
- [ ] queue 상태 초기화
- [ ] mutex 초기화
- [ ] condition variable 초기화
- [ ] `pthread_create`로 worker 생성

체크 질문:

- [ ] worker thread는 요청이 들어올 때 생성되는가, 서버 시작 시 생성되는가?
- [ ] worker 생성 중 일부가 실패하면 어떻게 정리하는가?
- [ ] `handler`가 NULL이면 왜 초기화를 실패시키는가?

답변 힌트:

```text
worker는 서버 시작 시 미리 생성됩니다. handler가 없으면 worker가 fd를 꺼내도 실제 처리할 함수가 없으므로 초기화가 실패해야 합니다.
```

## 4-4. bounded circular queue 이해

파일:

- [ ] `server/thread_pool.c`

확인할 함수:

- [ ] `thread_pool_submit`
- [ ] `thread_pool_worker_main`

queue에 넣는 흐름:

```text
mutex lock
-> stop_requested가 false이고 size < queue_capacity인지 확인
-> jobs[tail]에 client_fd 저장
-> tail = (tail + 1) % queue_capacity
-> size++
-> pthread_cond_signal
-> mutex unlock
```

queue에서 꺼내는 흐름:

```text
mutex lock
-> queue가 비어 있으면 cond_wait
-> jobs[head]에서 client_fd 꺼냄
-> head = (head + 1) % queue_capacity
-> size--
-> mutex unlock
-> handler 호출
```

체크 질문:

- [ ] 왜 `head`와 `tail`에 `% queue_capacity`를 쓰는가?
- [ ] queue가 가득 차면 submit은 어떻게 되는가?
- [ ] queue 상태를 바꿀 때 왜 mutex가 필요한가?

답변 힌트:

```text
배열을 원형으로 재사용하기 위해 modulo 연산을 씁니다. queue가 가득 차면 submit은 실패하고, server_run은 503 Server is busy 응답을 보냅니다.
```

## 4-5. condition variable 이해

파일:

- [ ] `server/thread_pool.c`

확인할 코드:

- [ ] `pthread_cond_wait`
- [ ] `pthread_cond_signal`
- [ ] `pthread_cond_broadcast`

worker 대기 흐름:

```text
while (!stop_requested && size == 0) {
    pthread_cond_wait(&pool->cond, &pool->mutex);
}
```

의미:

- [ ] queue가 비어 있으면 worker는 busy waiting하지 않는다.
- [ ] 새 job이 들어오면 submit이 `pthread_cond_signal`로 worker 하나를 깨운다.
- [ ] shutdown 때는 `pthread_cond_broadcast`로 모든 worker를 깨운다.

체크 질문:

- [ ] worker가 while loop로 조건을 다시 확인하는 이유는 무엇인가?
- [ ] `signal`과 `broadcast`는 언제 쓰이는가?
- [ ] condition variable이 없으면 worker는 어떻게 기다려야 하는가?

답변 힌트:

```text
condition variable은 queue가 비어 있을 때 worker를 잠들게 해서 CPU를 낭비하지 않게 합니다. 새 job이 들어오면 필요한 worker를 깨웁니다.
```

## 4-6. shutdown 흐름 이해

파일:

- [ ] `server_main.c`
- [ ] `server/server.c`
- [ ] `server/thread_pool.c`

확인할 흐름:

- [ ] `SIGINT` 또는 `SIGTERM`
- [ ] `handle_signal`
- [ ] `server_signal_shutdown`
- [ ] `g_shutdown_requested = 1`
- [ ] accept loop 종료
- [ ] `thread_pool_shutdown`
- [ ] `thread_pool_destroy`
- [ ] worker join

체크 질문:

- [ ] shutdown flag는 어떤 타입인가?
- [ ] signal handler 안에서 복잡한 정리 작업을 직접 하지 않는 이유는 무엇인가?
- [ ] shutdown 시 queue에 남은 작업은 어떻게 되는가?

답변 힌트:

```text
signal handler는 shutdown flag만 세우고, 실제 정리는 정상 control flow에서 수행합니다. worker는 stop_requested가 true여도 queue에 남은 작업이 있으면 처리한 뒤 종료합니다.
```

## 4-7. DB mutex 이해

파일:

- [ ] `server/server.c`
- [ ] `server/api.c`

확인할 코드:

- [ ] `pthread_mutex_init(&server->db_mutex, NULL)`
- [ ] `api_handle_query(server->table, &server->db_mutex, request.body, &api_result)`
- [ ] `pthread_mutex_lock(db_mutex)`
- [ ] `sql_execute(table, sql)`
- [ ] `pthread_mutex_unlock(db_mutex)`

왜 필요한가:

- [ ] 모든 요청이 하나의 `Table`을 공유한다.
- [ ] INSERT는 `next_id`를 증가시킨다.
- [ ] INSERT는 `rows` 배열을 변경한다.
- [ ] INSERT는 B+Tree를 변경한다.
- [ ] SELECT도 변경 중인 자료구조를 읽을 수 있다.
- [ ] SQL 엔진 자체는 thread-safe하게 작성되어 있지 않다.

DB mutex가 없을 때 가능한 문제:

- [ ] 두 INSERT가 같은 `next_id`를 받을 수 있다.
- [ ] `rows` 배열 realloc 중 다른 thread가 읽을 수 있다.
- [ ] B+Tree split 중 다른 thread가 검색할 수 있다.
- [ ] SELECT 결과가 중간 상태를 볼 수 있다.

체크 질문:

- [ ] thread pool mutex는 무엇을 보호하는가?
- [ ] DB mutex는 무엇을 보호하는가?
- [ ] 하나의 mutex로 모든 DB 요청을 막는 것의 장단점은 무엇인가?

답변 힌트:

```text
thread pool mutex는 job queue의 head, tail, size를 보호합니다. DB mutex는 shared Table, rows 배열, next_id, B+Tree를 보호합니다.
```

## 4-8. 병렬성과 직렬성 정확히 말하기

정확한 설명:

```text
서버는 여러 worker thread로 HTTP 요청을 동시에 받을 수 있습니다. 하지만 공유 DB 엔진은 하나의 mutex로 보호되므로 실제 SQL 실행 구간은 한 번에 하나의 요청만 들어갑니다.
```

병렬인 부분:

- [ ] 여러 connection이 accept되고 queue에 들어갈 수 있다.
- [ ] 여러 worker가 서로 다른 client fd를 처리할 수 있다.
- [ ] HTTP request parsing은 worker별로 수행된다.
- [ ] HTTP response 전송도 worker별로 수행된다.

직렬화되는 부분:

- [ ] `api_handle_query` 안의 DB mutex 구간
- [ ] `sql_execute`
- [ ] `SQLResult` JSON 변환

체크 질문:

- [ ] 이 구조를 “완전 병렬 DBMS”라고 말해도 되는가?
- [ ] 왜 DB 실행 구간을 직렬화했는가?
- [ ] 개선한다면 어떤 lock으로 바꿀 수 있는가?

답변 힌트:

```text
완전 병렬 DBMS는 아닙니다. 안전성을 우선해서 DB 실행 구간은 직렬화했고, 개선한다면 SELECT는 read lock, INSERT는 write lock을 잡는 rwlock 구조로 바꿀 수 있습니다.
```

## Q&A 답변 템플릿

질문: 왜 요청마다 새 thread를 만들지 않았나요?

```text
요청마다 thread를 만들면 생성/해제 비용이 크고 요청 폭주 시 thread 수가 무제한으로 늘어날 수 있습니다. 그래서 미리 worker를 4개 만들어두고 queue에 들어온 connection을 처리하게 했습니다.
```

질문: 왜 bounded queue가 필요한가요?

```text
서버가 감당할 수 있는 요청 수를 제한하기 위해서입니다. queue가 무한이면 메모리 사용량이 계속 늘 수 있고, 현재 구현은 queue가 가득 차면 503으로 거절합니다.
```

질문: 왜 DB mutex가 필요한가요?

```text
모든 요청이 하나의 in-memory Table을 공유하고, INSERT 시 next_id, row 배열, B+Tree가 변경됩니다. 동시에 접근하면 race condition이 생길 수 있어서 전역 mutex로 보호합니다.
```

질문: 이 구조에서 진짜 병렬인가요?

```text
네트워크 요청 파싱과 응답 처리는 여러 worker에서 병렬로 처리됩니다. 다만 실제 DB 접근은 하나의 mutex로 보호되기 때문에 DB 실행 구간은 직렬화됩니다.
```

## 완료 기준

- [ ] thread pool의 목적을 설명할 수 있다.
- [ ] circular queue의 `head`, `tail`, `size`를 설명할 수 있다.
- [ ] condition variable로 worker가 잠들고 깨어나는 흐름을 설명할 수 있다.
- [ ] queue full 시 503이 반환되는 흐름을 설명할 수 있다.
- [ ] thread pool mutex와 DB mutex의 차이를 설명할 수 있다.
- [ ] DB mutex가 없을 때 생길 수 있는 race condition을 예로 들 수 있다.
- [ ] 현재 구조의 한계와 `rwlock` 개선 방향을 설명할 수 있다.
