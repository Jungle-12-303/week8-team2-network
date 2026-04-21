# 12. Graceful Shutdown Strategy

## 목적

이 문서는 종료 처리 방식 두 가지를 비교하고, 이 프로젝트에 맞는 최종 선택을 정한다.

## 왜 이 문서가 필요한가

동시성 서버는 시작보다 종료가 더 어렵습니다.

특히 worker thread가 `pthread_cond_wait()`로 잠들어 있을 때, main thread가 어떻게 안전하게 종료 신호를 전달할지 결정해야 합니다.

이 프로젝트에서는 아래 두 가지 방식이 후보입니다.

- sentinel 방식
- 종료 플래그 방식

## sentinel 방식

sentinel 방식은 큐에 특별한 종료용 job을 넣어서 worker를 깨우는 방식입니다.

### 동작 방식

1. main thread가 종료 신호를 받는다
2. 종료용 sentinel job을 큐에 넣는다
3. worker가 sentinel job을 꺼낸다
4. worker는 종료 루프로 빠져나간다
5. 모든 worker가 빠지면 `pthread_join()`으로 정리한다

### 장점

- 구현이 직관적이다
- worker가 큐를 통해 자연스럽게 종료 신호를 받는다
- worker loop 안에서 종료 조건을 처리할 수 있다

### 단점

- job 큐 의미에 "진짜 작업"과 "종료 신호"가 섞인다
- sentinel 개수를 worker 수만큼 관리해야 한다
- 큐가 꽉 찬 상태에서 sentinel을 넣는 방법을 따로 정해야 한다
- job 구조에 `client_fd = -1` 같은 특수값이 들어가면 의미가 섞인다

## 종료 플래그 방식

종료 플래그 방식은 worker가 공유 종료 상태를 확인하고 빠져나오는 방식입니다.

### 동작 방식

1. main thread가 종료 신호를 받는다
2. 공유 종료 플래그를 true로 설정한다
3. `pthread_cond_broadcast()`로 대기 중인 worker를 모두 깨운다
4. worker는 종료 플래그를 확인한다
5. 종료 플래그가 켜져 있으면 안전하게 루프를 빠져나간다
6. `pthread_join()`으로 정리한다

### 장점

- 큐에 종료용 job을 넣지 않아도 된다
- job queue는 진짜 client request만 다룬다
- 종료 조건이 전역 상태로 분리되어 의미가 깔끔하다
- 큐가 가득 찬 상태와 종료 신호를 더 쉽게 분리할 수 있다

### 단점

- worker loop와 shared state를 조금 더 조심해서 봐야 한다
- 종료 플래그와 condition variable의 관계를 정확히 맞춰야 한다
- 락 순서와 wake-up 규칙을 잘 문서화해야 한다

## 추천

이 프로젝트에는 **종료 플래그 + `pthread_cond_broadcast()` + `pthread_join()` 방식**을 추천합니다.

### 추천 이유

- job queue의 의미가 깨끗합니다
- `client_fd = -1` 같은 특수 job이 필요 없습니다
- API / DB / concurrency 역할이 분리된 구조와 잘 맞습니다
- 지금 계획의 핵심 가치인 "책임 분리"와 잘 맞습니다
- 6페르소나 병렬 구현에서도 종료 정책을 한 곳에서 보기 쉽습니다

## 구현 원칙

- 새 요청 수락은 종료 플래그가 켜지면 멈춘다
- 이미 큐에 들어간 job은 정책에 따라 처리하거나 마무리한다
- worker는 종료 플래그를 확인한 뒤 정상 종료한다
- 종료 시에는 모든 worker를 깨우고 `pthread_join()`으로 마무리한다

## 이 프로젝트의 최종 선택

이 프로젝트는 **종료 플래그 방식**을 최종 기본안으로 사용합니다.

sentinel 방식은 대안으로만 남겨두고, 실제 구현과 문서는 종료 플래그 방식을 기준으로 맞춥니다.
