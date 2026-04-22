# Mini DBMS Server - thread pool 동시성 학습 노트 (CYB)

> 이 문서는 `server/thread_pool.c`를 읽을 때 나오는 핵심 질문,  
> 특히 `mutex`, `cond_wait`, `cond_signal`, `stop_requested`, `size/head/tail`의 의미를 이해하기 위한 보조 자료다.

---

## 1. 먼저 전체 구조를 한 문장으로

이 thread pool은:

- 메인 스레드가 `client_fd`를 큐에 넣고
- 워커 스레드가 큐에서 `client_fd`를 꺼내 처리하는
- 전형적인 producer-consumer 구조다.

코드 기준:

- producer: [thread_pool_submit()](/Users/choeyeongbin/week8-team2-network/server/thread_pool.c:93)
- consumer: [thread_pool_worker_main()](/Users/choeyeongbin/week8-team2-network/server/thread_pool.c:7)

---

## 2. `ThreadPool` 구조체에서 꼭 봐야 하는 필드

정의 위치:

- [thread_pool.h](/Users/choeyeongbin/week8-team2-network/server/thread_pool.h:13)

핵심 필드는 다음 다섯 개다.

### `jobs`

실제 작업이 들어 있는 배열이다.  
이 프로젝트에서는 작업 단위가 `client_fd` 하나다.

```c
typedef struct ThreadPoolJob {
    int client_fd;
} ThreadPoolJob;
```

### `head`

워커가 다음으로 꺼낼 위치다.

### `tail`

메인 스레드가 다음으로 넣을 위치다.

### `size`

현재 큐에 몇 개의 작업이 들어 있는지 나타낸다.

### `mutex`, `cond`

- `mutex`: 큐 상태를 안전하게 보호
- `cond`: 큐가 비어 있을 때 워커를 재우고, 새 작업이 생기면 깨우는 용도

---

## 3. 왜 이게 producer-consumer인가

### producer

메인 스레드는 `accept()`로 받은 `client_fd`를 큐에 넣는다.

```c
pool->jobs[pool->tail].client_fd = client_fd;
pool->tail = (pool->tail + 1) % pool->queue_capacity;
pool->size++;
```

즉 메인 스레드는 “작업 생산자”다.

### consumer

워커 스레드는 큐에서 작업을 꺼낸다.

```c
client_fd = pool->jobs[pool->head].client_fd;
pool->head = (pool->head + 1) % pool->queue_capacity;
pool->size--;
```

즉 워커는 “작업 소비자”다.

---

## 4. `cond_wait`는 왜 필요한가

코드 위치:

- [thread_pool.c](/Users/choeyeongbin/week8-team2-network/server/thread_pool.c:13)

```c
pthread_mutex_lock(&pool->mutex);
while (!pool->stop_requested && pool->size == 0) {
    pthread_cond_wait(&pool->cond, &pool->mutex);
}
```

이 코드는 이렇게 읽으면 된다.

> "지금 종료 요청도 없고, 큐도 비어 있으면 일할 게 없으니 잠들어라."

### 왜 `while`이지 `if`가 아닐까

이건 매우 중요하다.

`pthread_cond_wait()`에서 깨어났다고 해서

- 진짜로 작업이 생겼다는 보장은 없고
- 다른 워커가 먼저 가져갔을 수도 있고
- spuriously wakeup될 수도 있다

그래서 깨어난 뒤 조건을 다시 검사해야 한다.  
그 때문에 `if`가 아니라 `while`을 쓴다.

---

## 5. `cond_signal`은 왜 필요한가

코드 위치:

- [thread_pool.c](/Users/choeyeongbin/week8-team2-network/server/thread_pool.c:100)

```c
if (!pool->stop_requested && pool->size < pool->queue_capacity) {
    pool->jobs[pool->tail].client_fd = client_fd;
    pool->tail = (pool->tail + 1) % pool->queue_capacity;
    pool->size++;
    accepted = 1;
    pthread_cond_signal(&pool->cond);
}
```

이 코드는 이렇게 읽으면 된다.

> "새 작업을 하나 넣었으니, 자고 있는 워커 하나를 깨워라."

### 왜 `signal`이지 `broadcast`가 아닐까

여기서는 작업 하나가 들어왔기 때문에
워커 하나만 깨우면 충분하다.

만약 작업 하나 넣을 때마다 모든 워커를 깨우면:

- 여러 워커가 한 번에 깨어나고
- 결국 하나만 일을 가져가고
- 나머지는 다시 잠드는 비효율이 생긴다.

그래서 submit 단계에서는 `pthread_cond_signal()`이 맞다.

---

## 6. `stop_requested`는 왜 필요한가

관련 위치:

- worker loop: [thread_pool.c](/Users/choeyeongbin/week8-team2-network/server/thread_pool.c:14)
- shutdown: [thread_pool.c](/Users/choeyeongbin/week8-team2-network/server/thread_pool.c:113)

worker 쪽 종료 조건:

```c
if (pool->stop_requested && pool->size == 0) {
    pthread_mutex_unlock(&pool->mutex);
    break;
}
```

shutdown 쪽:

```c
pthread_mutex_lock(&pool->mutex);
pool->stop_requested = 1;
pthread_cond_broadcast(&pool->cond);
pthread_mutex_unlock(&pool->mutex);
```

### 의미

`stop_requested`는 이렇게 동작한다.

- 더 이상 새 작업을 정상적으로 받지 않게 하고
- 자고 있는 워커들을 깨워서
- 큐가 비었으면 루프를 빠져나가 종료하게 만든다.

### 왜 `broadcast`를 쓰나

shutdown 시에는 특정 워커 하나만 깨우면 안 된다.

- 여러 워커가 모두 잠들어 있을 수 있고
- 모두 종료 조건을 확인해야 하므로
- `pthread_cond_broadcast()`로 전부 깨워야 한다.

즉:

- submit: `signal`
- shutdown: `broadcast`

이다.

---

## 7. 핵심 질문: mutex 없이 `size/head/tail`을 수정하면 무슨 일이 생기나

이 질문이 가장 중요하다.

결론부터 말하면:

> 큐 상태가 깨지고, 작업 유실/중복/메모리 오염/무한 대기가 발생할 수 있다.

### 이유 1. `tail` 경쟁

메인 스레드가 작업 두 개를 거의 동시에 넣는 상황을 상상해 보자.  
현재 구조에선 submit이 보통 메인 스레드 하나에서 호출되지만,
원칙적으로 보호가 없으면 동일한 공유 상태 변경은 위험하다.

예를 들어:

- 두 흐름이 같은 `tail = 3`을 읽음
- 둘 다 `jobs[3]`에 쓰려고 함
- 둘 다 `tail = 4`로 갱신

그러면 작업 하나가 덮어써질 수 있다.

### 이유 2. `head` 경쟁

여러 워커가 동시에 같은 `head`를 읽으면:

- 같은 작업을 두 워커가 중복 처리할 수 있다
- 혹은 하나가 가져간 뒤 다른 하나도 같은 슬롯을 읽을 수 있다

### 이유 3. `size` 불일치

`size++`, `size--`는 원자적 연산이 아니다.

예를 들어:

- 한 스레드가 `size = 5`를 읽고 `6`으로 쓰려는 중
- 다른 스레드가 `size = 5`를 읽고 `4`로 쓰려는 중

마지막 쓰기만 남으면

- 실제 큐 상태는 5인데
- `size`는 4 또는 6이 되어 버릴 수 있다.

그러면:

- 큐가 비어 있는데 안 비었다고 믿고 읽으려 하거나
- 큐가 찼는데 안 찼다고 믿고 덮어쓸 수 있다.

### 이유 4. 조건 변수와 상태가 어긋남

조건 변수는 “상태 변화”와 같이 써야 한다.

즉:

- mutex로 상태를 변경하고
- 그 상태를 기준으로 `cond_wait`, `cond_signal`을 써야 한다.

mutex 없이 상태를 바꾸면

- 워커는 `size == 0`이라고 믿고 잠들고
- 실제로는 작업이 들어왔는데 못 깨어날 수 있다

즉 **lost wakeup에 준하는 문제**가 생길 수 있다.

---

## 8. 지금 코드가 왜 안전한가

이 코드가 비교적 안전한 이유는:

- `head`, `tail`, `size`, `stop_requested`
- 이 네 값에 접근할 때 항상 `pool->mutex`를 사용하기 때문이다.

예:

- worker: [thread_pool.c](/Users/choeyeongbin/week8-team2-network/server/thread_pool.c:13)
- submit: [thread_pool.c](/Users/choeyeongbin/week8-team2-network/server/thread_pool.c:100)
- shutdown: [thread_pool.c](/Users/choeyeongbin/week8-team2-network/server/thread_pool.c:118)

즉 큐 상태를 바꾸는 모든 경로가 동일한 mutex 아래에 있다.

---

## 9. 이 파일을 읽을 때 꼭 스스로 답해봐야 할 질문

### 질문 1. worker가 왜 `while (!stop_requested && size == 0)` 조건으로 잠들까

답:

- 일할 게 없으면 잠들고
- shutdown이면 더 이상 대기하지 않고 종료해야 하기 때문이다.

### 질문 2. submit은 왜 `signal`, shutdown은 왜 `broadcast`일까

답:

- 새 작업 하나에는 워커 하나면 충분하고
- 종료는 모든 워커가 알아야 하기 때문이다.

### 질문 3. 왜 `head/tail/size`는 항상 mutex 아래에서 수정해야 할까

답:

- 큐의 일관성이 깨지면 producer-consumer 구조 전체가 무너진다.

### 질문 4. 왜 worker가 handler 호출 전에 mutex를 풀까

답:

- 요청 처리(`http_read_request`, `api_handle_query`)는 오래 걸릴 수 있다.
- 그동안 mutex를 잡고 있으면 submit도, 다른 worker도 큐를 못 건드린다.
- 그래서 큐에서 작업만 꺼낸 뒤 바로 unlock하는 것이 맞다.

---

## 10. 한 문장 결론

이 thread pool의 본질은:

> mutex로 보호되는 원형 큐 위에서,  
> 메인 스레드는 `cond_signal`로 워커를 깨우고,  
> 워커는 `cond_wait`로 잠들었다가 작업이 생기면 처리하는  
> producer-consumer 동기화 패턴이다.
