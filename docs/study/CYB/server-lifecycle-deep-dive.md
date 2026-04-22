# Mini DBMS Server - 서버 동작 흐름 상세 해설 (CYB)

> `project-study-guide.md`가 전체 그림을 빠르게 잡기 위한 문서라면, 이 문서는 서버의 각 구간을 실제 코드 기준으로 자세히 풀어 설명하기 위한 문서다.

---

## 1. 전체 흐름 한 줄 요약

```text
main() -> 시그널 핸들러 설치 -> server_create()
      -> table / mutex / thread pool / listen socket 준비
      -> server_run()의 accept 루프
      -> thread_pool_submit(client_fd)
      -> worker가 server_handle_client() 수행
      -> HTTP 파싱 -> SQL 실행 -> 응답 전송
      -> shutdown 요청 시 thread_pool_shutdown() -> server_destroy()
```

---

## 2. 구간별 상세 설명

### 구간 1. 프로세스 시작과 포트 파싱

**코드 위치**

- [server_main.c](/Users/choeyeongbin/week8-team2-network/server_main.c:32)

**무슨 일을 하나**

- 프로그램 진입점인 `main()`이 실행된다.
- 기본 포트는 `8080`이다.
- 사용자가 인자를 넘기면 `argv[1]`을 포트 문자열로 읽는다.
- `strtoul()`로 숫자 변환을 시도하고, 잘못된 포트면 즉시 종료한다.

**왜 필요한가**

- 서버는 어느 포트에서 연결을 받을지 알아야 `bind()`할 수 있다.
- `"abc"` 같은 값이나 `0`, `65535` 초과 값은 TCP 포트로 유효하지 않다.

**핵심 코드**

```c
port = strtoul(argv[1], &end_ptr, 10);
if (end_ptr == argv[1] || *end_ptr != '\0' || port == 0 || port > 65535) {
    fprintf(stderr, "Invalid port: %s\n", argv[1]);
    return 1;
}
```

**핵심 변수**

- `port`: 최종적으로 서버가 사용할 포트 번호
- `end_ptr`: 문자열이 끝까지 정상적으로 숫자로 파싱되었는지 검사하는 포인터

---

### 구간 2. 시그널 핸들러 설치

**코드 위치**

- [server_main.c](/Users/choeyeongbin/week8-team2-network/server_main.c:8)
- [server_main.c](/Users/choeyeongbin/week8-team2-network/server_main.c:13)
- [server/server.c](/Users/choeyeongbin/week8-team2-network/server/server.c:28)

**무슨 일을 하나**

- `SIGINT`, `SIGTERM`을 받았을 때 `handle_signal()`이 실행되도록 등록한다.
- 핸들러 내부에서는 복잡한 정리를 직접 하지 않고 `server_signal_shutdown()`만 호출한다.
- `server_signal_shutdown()`은 전역 플래그 `g_shutdown_requested`를 `1`로 바꾼다.

**왜 이렇게 구현했나**

- 시그널 핸들러 안에서는 할 수 있는 일이 제한적이다.
- 그래서 “지금 당장 종료 정리”를 하지 않고, “종료 요청이 들어왔음”만 표시한다.
- 이후 메인 루프가 그 플래그를 보고 정상적인 종료 경로로 이동한다.

**핵심 개념**

- `sigaction()`: 시그널 핸들러 등록
- `volatile sig_atomic_t`: 시그널 컨텍스트에서도 비교적 안전하게 사용하는 플래그 타입

---

### 구간 3. `ServerConfig` 설정

**코드 위치**

- [server_main.c](/Users/choeyeongbin/week8-team2-network/server_main.c:51)

**무슨 일을 하나**

- `config.port`
- `config.worker_count = 4`
- `config.queue_capacity = 16`
- `config.backlog = 32`

를 채운 뒤 `server_create(&config)`에 넘긴다.

**이 값들의 의미**

- `port`: 몇 번 포트에서 받을지
- `worker_count`: 동시에 요청을 처리할 워커 수
- `queue_capacity`: accept된 연결을 임시 적재할 큐 크기
- `backlog`: `listen()`이 커널에게 넘기는 대기열 힌트

**주의할 점**

- `queue_capacity`와 `backlog`는 다른 개념이다.
- `backlog`는 커널 레벨의 listen 대기열 크기이고,
- `queue_capacity`는 애플리케이션 내부의 thread pool 큐 크기다.

---

### 구간 4. `server_create()`로 서버 객체 초기화

**코드 위치**

- [server/server.c](/Users/choeyeongbin/week8-team2-network/server/server.c:151)

**무슨 일을 하나**

`server_create()`는 서버 실행에 필요한 핵심 자원을 한 번에 준비한다.

순서는 다음과 같다.

1. `Server` 구조체 할당
2. `table_create()`로 인메모리 테이블 생성
3. `pthread_mutex_init()`로 `db_mutex` 초기화
4. `thread_pool_init()`로 워커 스레드들과 큐 생성
5. `server_make_listen_socket()`로 `listen_fd` 생성

**왜 이 순서인가**

- 뒤 단계가 앞 단계 자원에 의존한다.
- 예를 들어 thread pool을 만들기 전에는 `server` 구조체와 핸들러 컨텍스트가 준비돼 있어야 한다.
- listen socket을 만들기 전에 서버 오브젝트 전체가 만들어져 있어야 이후 실행이 자연스럽다.

**실패 시 처리**

- 어느 단계에서 실패하든 그 전까지 생성한 자원을 역순으로 해제한다.
- 이 패턴은 누수 없이 초기화를 다루는 전형적인 C 스타일이다.

---

### 구간 5. `table_create()`로 데이터 저장소 준비

**코드 위치**

- [server/server.c](/Users/choeyeongbin/week8-team2-network/server/server.c:167)

**무슨 일을 하나**

- 서버가 사용할 `Table *table`을 만든다.
- 이 객체는 HTTP 요청이 들어왔을 때 실제 SQL이 읽고 쓰는 인메모리 데이터 저장소다.

**왜 필요한가**

- 이 프로젝트의 DBMS는 영속 스토리지가 아니라 메모리 테이블 기반이다.
- 따라서 서버가 시작될 때 메모리 상의 users 테이블이 준비되어 있어야 한다.

**연결되는 다음 단계**

- 이후 `api_handle_query()`에서 `sql_execute(table, sql)`로 전달된다.

---

### 구간 6. `db_mutex` 초기화

**코드 위치**

- [server/server.c](/Users/choeyeongbin/week8-team2-network/server/server.c:173)

**무슨 일을 하나**

- `pthread_mutex_init(&server->db_mutex, NULL)`를 호출한다.

**왜 필요한가**

- 요청은 여러 워커 스레드가 동시에 처리할 수 있다.
- 그런데 `table`은 공유 자원이다.
- 따라서 동시에 여러 스레드가 SQL을 실행하면 데이터 경쟁이 생길 수 있다.

**현재 구조에서 역할**

- `api_handle_query()`가 SQL 실행 전 락을 잡고, 실행 후 해제한다.

---

### 구간 7. `thread_pool_init()`로 워커 스레드 생성

**코드 위치**

- [server/server.c](/Users/choeyeongbin/week8-team2-network/server/server.c:180)
- [server/thread_pool.c](/Users/choeyeongbin/week8-team2-network/server/thread_pool.c:37)

**무슨 일을 하나**

- `workers` 배열 할당
- `jobs` 배열 할당
- `mutex`, `cond` 초기화
- `worker_count` 개수만큼 `pthread_create()`로 워커 생성

**thread pool의 핵심 구조**

- 메인 스레드: `accept()`한 `client_fd`를 큐에 넣는 producer
- 워커 스레드: 큐에서 `client_fd`를 꺼내 처리하는 consumer

**워커 메인 루프**

- [thread_pool.c](/Users/choeyeongbin/week8-team2-network/server/thread_pool.c:7)

워커는 다음 루프를 반복한다.

1. 큐가 비어 있으면 `pthread_cond_wait()`로 잠듦
2. 작업이 생기면 `client_fd` 하나 꺼냄
3. `handler(pool->context, client_fd)` 호출
4. 끝나면 `close(client_fd)`

**왜 좋은가**

- 매 요청마다 새 스레드를 만들지 않아도 된다.
- 스레드 생성/파괴 비용을 줄이고 구조를 단순하게 유지할 수 있다.

---

### 구간 8. `server_make_listen_socket()`에서 listen 준비

**코드 위치**

- [server/server.c](/Users/choeyeongbin/week8-team2-network/server/server.c:38)

**무슨 일을 하나**

1. `socket(AF_INET, SOCK_STREAM, 0)`
2. `setsockopt(... SO_REUSEADDR ...)`
3. 주소 구조체 설정
4. `bind()`
5. `listen()`

**핵심 포인트**

- 이 함수가 성공해야 비로소 서버가 외부의 TCP `connect()`를 받을 준비가 끝난다.
- 즉, `curl`이 연결을 시도하기 전에 서버는 이미 여기까지 끝내고 있어야 한다.

**변수 의미**

- `listen_fd`: 새 연결을 기다리는 서버 소켓
- `address.sin_port = htons(port)`: 포트를 네트워크 바이트 오더로 변환
- `INADDR_ANY`: 모든 로컬 인터페이스에서 연결 수락

**왜 `SO_REUSEADDR`를 쓰나**

- 서버를 빠르게 재시작할 때 포트가 잠시 묶여 있는 문제를 줄여준다.

---

### 구간 9. `server_run()`에서 accept 루프 시작

**코드 위치**

- [server/server.c](/Users/choeyeongbin/week8-team2-network/server/server.c:202)

**무슨 일을 하나**

- `while (!server_shutdown_requested())` 루프를 돈다.
- `accept(server->listen_fd, ...)`로 새 연결이 들어오길 기다린다.

**핵심 개념**

- `listen_fd`는 계속 살아 있는 “대기용” 소켓이다.
- `accept()`가 성공하면 요청 처리용 `client_fd`가 새로 생긴다.
- 즉 `listen_fd`와 `client_fd`는 역할이 다르다.

**에러 처리**

- `EINTR`이면 시그널 인터럽트로 보고 다시 루프를 돈다.
- shutdown 요청이 들어온 상태면 루프를 빠져나온다.

---

### 구간 10. `thread_pool_submit(client_fd)`로 작업 전달

**코드 위치**

- [server/server.c](/Users/choeyeongbin/week8-team2-network/server/server.c:225)
- [server/thread_pool.c](/Users/choeyeongbin/week8-team2-network/server/thread_pool.c:93)

**무슨 일을 하나**

- 메인 스레드가 방금 받은 `client_fd`를 thread pool 큐에 넣는다.

**성공하면**

- `jobs[tail]`에 `client_fd`를 넣고
- `tail`을 이동시키고
- `size++`
- `pthread_cond_signal()`로 워커 하나를 깨운다.

**실패하면**

- 큐가 가득 찬 상태다.
- `server.c`가 직접 `503` JSON 응답을 보내고 `close(client_fd)` 한다.

**왜 즉시 503인가**

- 큐가 가득 찼는데도 더 받으면 애플리케이션 내부에서 감당할 수 없다.
- 그래서 “지금 바쁨”을 명시적으로 알려준다.

---

### 구간 11. 워커가 `server_handle_client()` 실행

**코드 위치**

- [server/thread_pool.c](/Users/choeyeongbin/week8-team2-network/server/thread_pool.c:28)
- [server/server.c](/Users/choeyeongbin/week8-team2-network/server/server.c:117)

**무슨 일을 하나**

- 워커는 큐에서 꺼낸 `client_fd`를 `server_handle_client()`에 넘긴다.
- 이 함수가 “연결 1건 처리”의 실제 진입점이다.

**함수 내부 역할**

1. `http_read_request()`로 요청 파싱
2. 실패하면 적절한 HTTP 에러 응답 작성
3. 성공하면 `api_handle_query()` 호출
4. 결과 JSON을 `http_send_response()`로 전송

**중요한 점**

- `server_handle_client()` 자체는 소켓을 닫지 않는다.
- 닫는 작업은 워커 루프 바깥에서 `close(client_fd)`로 수행된다.

---

### 구간 12. `http_read_request()`로 HTTP 파싱

**코드 위치**

- [server/server.c](/Users/choeyeongbin/week8-team2-network/server/server.c:126)
- 구현 본체는 `server/http.c`

**무슨 일을 하나**

- `recv()`로 헤더를 읽음
- `\r\n\r\n`을 찾아 헤더와 바디 경계를 찾음
- method/path/version 파싱
- `POST`만 허용
- `/query`만 허용
- `Content-Length` 파싱
- body를 끝까지 읽음

**왜 필요한가**

- TCP는 바이트 스트림이지 “HTTP 요청 객체”가 아니다.
- 따라서 서버가 직접 바이트를 읽어 의미 있는 HTTP 구조로 바꿔야 한다.

---

### 구간 13. `api_handle_query()`와 SQL 실행

**코드 위치**

- [server/server.c](/Users/choeyeongbin/week8-team2-network/server/server.c:138)
- 구현 본체는 `server/api.c`

**무슨 일을 하나**

- `db_mutex`를 잡고
- `sql_execute(table, sql)` 실행
- 결과를 JSON으로 직렬화
- 응답 상태와 body를 반환

**왜 API 레이어가 필요한가**

- HTTP 파싱과 SQL 실행을 분리하기 위해서다.
- `http.c`는 HTTP 문법을, `sql.c`는 SQL 문법을, `api.c`는 둘 사이 연결을 맡는다.

---

### 구간 14. 응답 전송과 연결 종료

**코드 위치**

- [server/server.c](/Users/choeyeongbin/week8-team2-network/server/server.c:147)
- [server/thread_pool.c](/Users/choeyeongbin/week8-team2-network/server/thread_pool.c:31)

**무슨 일을 하나**

- `http_send_response()`로 JSON 응답을 보낸다.
- 그 다음 워커 루프가 `close(client_fd)`를 호출한다.

**현재 구조의 의미**

- keep-alive를 유지하지 않는다.
- 연결 1건, 요청 1건, 응답 1건 처리 후 소켓을 닫는다.

즉 현재 서버는 사실상 `request per connection` 구조다.

---

### 구간 15. shutdown 요청과 종료 경로

**코드 위치**

- [server/server.c](/Users/choeyeongbin/week8-team2-network/server/server.c:210)
- [server/thread_pool.c](/Users/choeyeongbin/week8-team2-network/server/thread_pool.c:113)
- [server/server.c](/Users/choeyeongbin/week8-team2-network/server/server.c:239)

**무슨 일을 하나**

- 시그널 핸들러가 `g_shutdown_requested = 1`로 표시
- `server_run()` 루프가 이를 감지
- 루프 종료 후 `thread_pool_shutdown()` 호출
- `main()`으로 돌아와 `server_destroy()` 호출

**정리 순서**

1. 워커들 종료 요청
2. `listen_fd` close
3. `db_mutex` destroy
4. `table_destroy()`
5. `Server` 메모리 해제

**좋은 점**

- 즉시 강제 종료하지 않고, 구조화된 종료 경로를 따른다.
- 스레드 풀도 `thread_pool_destroy()`에서 join까지 수행한다.

---

## 3. 이 문서를 읽을 때 중요하게 봐야 할 질문

### 질문 1. 왜 `listen_fd`와 `client_fd`를 분리해야 하나?

- `listen_fd`는 계속 다음 연결을 기다리는 서버 소켓이다.
- `client_fd`는 `accept()`가 만들어 준 “연결 1건 처리용” 소켓이다.
- 이 둘을 구분해야 accept 루프의 의미가 보인다.

### 질문 2. 왜 메인 스레드가 직접 요청을 처리하지 않나?

- 메인 스레드가 요청 처리까지 맡으면 다음 연결을 빨리 받지 못한다.
- 그래서 메인 스레드는 연결 수락과 큐 적재만 하고,
- 실제 처리 비용이 큰 HTTP/SQL은 워커에게 넘긴다.

### 질문 3. thread pool의 본질은 무엇인가?

- 고정 개수의 worker threads
- shared job queue
- producer-consumer synchronization

이 세 가지의 조합이다.

### 질문 4. 지금 구조는 keep-alive 서버인가?

- 아니다.
- 현재는 요청 1건을 처리하고 나면 워커가 `close(client_fd)`를 호출한다.
- 따라서 지속 연결을 재사용하는 구조는 아니다.

---

## 4. 추천 읽기 순서

1. [server_main.c](/Users/choeyeongbin/week8-team2-network/server_main.c:32)
2. [server/server.c](/Users/choeyeongbin/week8-team2-network/server/server.c:151)
3. [server/server.c](/Users/choeyeongbin/week8-team2-network/server/server.c:38)
4. [server/server.c](/Users/choeyeongbin/week8-team2-network/server/server.c:202)
5. [server/thread_pool.c](/Users/choeyeongbin/week8-team2-network/server/thread_pool.c:7)
6. [server/server.c](/Users/choeyeongbin/week8-team2-network/server/server.c:117)
7. `server/http.c` -> `server/api.c` -> `sql_processor/sql.c`

이 순서로 읽으면 “서버 준비 -> 연결 수락 -> 작업 위임 -> 요청 처리 -> 응답 -> 종료” 흐름이 자연스럽게 이어진다.
