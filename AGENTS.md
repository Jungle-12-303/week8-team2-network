# 수요 코딩회 (수요일)

## 목적

- 단 하루, AI를 도구 삼아 팀별 프로젝트에 몰입합니다.
- 결과물뿐만 아니라 과정도 중요합니다. AI가 작성한 코드를 완벽히 내 것으로 소화해 그 원리를 설명할 수 있어야 합니다.
- 낯선 기능도 Top-down으로 빠르게 풀어내며 학습 민첩성을 극대화하고, 현업 레벨의 기술을 직접 부딪쳐보며 실무 감각을 깨웁니다.

## 요구사항

- 이번 주 과제는 미니 DBMS - API 서버를 구현하는 것입니다.
- 구현한 API를 통해 외부 클라이언트에서 DBMS 기능을 사용할 수 있어야 합니다.
- 스레드 풀(Thread Pool)을 구성하고, 요청이 들어올 때마다 스레드를 할당하여 SQL 요청을 병렬로 처리해야 합니다.
- 이전 차수에서 구현한 SQL 처리기와 B+ 트리 인덱스를 그대로 활용하여 내부 DB 엔진을 구성합니다.
- 구현 언어는 C 언어입니다.
- 학습보다는 구현이 우선입니다. AI 등을 적극적으로 활용하여 결과물을 만들어내는 것이 최우선 목표입니다.
- 다만, 만들어진 결과물 중 핵심 로직에 대해서는 이해하고 설명할 수 있어야 합니다.
- 코드를 직접 작성하기 어려운 경우, 전체 코드를 AI를 통해 생성해도 괜찮습니다. 다만, 생성된 코드를 반드시 학습하고 그 원리를 이해해야 합니다.
- 추가적으로, 다른 팀과의 차별점을 둘 수 있는 추가 구현 요소에 대해서도 함께 고민합니다.

## 중점 포인트

- 멀티 스레드 동시성 이슈
- 내부 DB 엔진과 외부 API 서버 사이 연결 설계
- API 서버 아키텍처

## 요구사항을 이해하기 위한 필수 개념

이 프로젝트는 단순한 C 서버가 아니라, 기존 SQL 처리기와 B+ 트리 인덱스를 외부 API로 사용할 수 있게 감싼 미니 DBMS입니다. 따라서 아래 개념은 구현 결과를 설명하기 위해 반드시 이해해야 합니다.

### 네트워크와 HTTP

- 소켓(Socket): 클라이언트와 서버가 데이터를 주고받는 통신 창구입니다.
- 파일 디스크립터(fd): C에서 소켓 연결을 읽고 쓰기 위해 사용하는 정수 핸들입니다.
- `socket`, `bind`, `listen`, `accept`: 서버가 포트를 열고 클라이언트 연결을 받는 기본 흐름입니다.
- HTTP 요청 구조: method, path, header, body로 구성됩니다.
- `Content-Length`: HTTP body를 어디까지 읽어야 하는지 알려주는 헤더입니다.
- HTTP 상태 코드: `200`, `400`, `404`, `405`, `413`, `500`, `503`처럼 요청 처리 결과를 표현합니다.

### API 서버 아키텍처

- Endpoint: 외부 클라이언트가 호출하는 API 주소입니다. 이 프로젝트는 `POST /query`를 사용합니다.
- Request/Response 계약: 클라이언트가 어떤 SQL을 보내고, 서버가 어떤 JSON을 돌려주는지 정한 약속입니다.
- 계층 분리: socket 처리, HTTP 파싱, API 응답 변환, DB 실행을 나누어 설계합니다.
- 오류 구분: HTTP 요청 오류와 SQL 실행 오류를 구분해서 응답해야 합니다.

### 스레드와 동시성

- 스레드(Thread): 한 프로세스 안에서 동시에 실행되는 작업 단위입니다.
- 병렬 처리: 여러 요청을 여러 worker thread가 동시에 처리하는 구조입니다.
- 스레드 풀(Thread Pool): 요청마다 새 스레드를 만들지 않고, 미리 만든 worker thread를 재사용하는 방식입니다.
- 작업 큐(Job Queue): accept된 client fd를 worker thread에게 넘기기 위해 잠시 저장하는 공간입니다.
- Producer-Consumer 구조: main thread는 job을 넣는 producer, worker thread는 job을 꺼내 처리하는 consumer입니다.
- Bounded Queue: 큐 크기를 제한하여 서버가 감당할 수 없는 요청을 무한히 쌓지 않게 합니다.
- Queue Full: 큐가 가득 찼을 때 `503 Service Unavailable`로 응답하는 상황입니다.

### 동기화와 공유 자원 보호

- Race Condition: 여러 스레드가 같은 데이터를 동시에 건드려 결과가 깨지는 문제입니다.
- Mutex: 한 번에 하나의 스레드만 공유 자원에 접근하게 만드는 잠금 도구입니다.
- Critical Section: mutex로 보호해야 하는 코드 구간입니다.
- Condition Variable: 큐가 비어 있을 때 worker가 기다리고, job이 들어왔을 때 깨어나게 하는 도구입니다.
- Deadlock: 서로 lock을 기다리며 영원히 멈추는 문제입니다.
- Lock 범위: DB mutex는 `sql_execute()`와 JSON 직렬화가 끝날 때까지 유지해야 합니다.
- Thread Safety: 기존 `Table`, `BPTree`, `Record`는 thread-safe하지 않으므로 mutex로 보호해야 합니다.

### 내부 DB 엔진

- SQL Parser: SQL 문자열을 해석해 `INSERT`, `SELECT`, `WHERE` 같은 동작으로 바꾸는 코드입니다.
- SQL Executor: 파싱된 SQL을 실제 `Table`에 적용하는 실행기입니다.
- `SQLResult`: SQL 실행 결과, row 수, 오류 상태, 메시지를 담는 구조입니다.
- `Table`: 서버 실행 중 공유되는 in-memory users 테이블입니다.
- `Record`: `id`, `name`, `age`를 담는 한 행의 데이터입니다.
- Auto Increment: INSERT할 때 id가 자동으로 증가하는 방식입니다.
- Index: 특정 컬럼을 빠르게 찾기 위한 자료구조입니다.
- B+ Tree: `id` 기반 검색과 range query를 빠르게 처리하기 위한 인덱스입니다.
- Linear Scan: `name`, `age` 조건처럼 인덱스가 없는 경우 모든 row를 순회하는 방식입니다.

### C 메모리 관리

- `malloc`, `calloc`, `realloc`, `free`: C에서 heap 메모리를 직접 관리하는 함수입니다.
- 메모리 소유권: 누가 할당한 메모리를 누가 해제해야 하는지 정해야 합니다.
- Memory Leak: 할당한 메모리를 해제하지 않아 누수되는 문제입니다.
- Use After Free: 이미 해제한 메모리를 다시 사용하는 위험한 버그입니다.
- Buffer Overflow: 정해진 배열 크기보다 많이 써서 메모리를 침범하는 문제입니다.
- JSON Buffer: SQL 결과를 클라이언트 응답 문자열로 만들기 위해 동적으로 커지는 버퍼입니다.

### 테스트와 품질

- Unit Test: B+Tree, Table, SQL 처리기 같은 내부 함수를 독립적으로 검증합니다.
- Smoke Test: 서버를 실행한 뒤 대표 API가 정상 동작하는지 빠르게 확인합니다.
- API 기능 테스트: `POST /query`가 INSERT, SELECT, 오류 SQL에 대해 올바른 응답을 주는지 확인합니다.
- 동시성 테스트: 여러 요청을 동시에 보내도 서버가 죽지 않고 id와 데이터가 깨지지 않는지 확인합니다.
- Edge Case: 잘못된 method, 잘못된 path, 큰 body, 잘못된 SQL, queue full 같은 예외 상황입니다.
- Graceful Shutdown: 종료 신호를 받았을 때 worker thread와 socket을 안전하게 정리하는 방식입니다.

### 빌드와 실행 환경

- Makefile: C 소스 파일을 어떤 옵션으로 컴파일하고 링크할지 정의합니다.
- `-pthread`: pthread 기반 스레드 코드를 링크하기 위한 컴파일 옵션입니다.
- Docker: 같은 환경에서 서버를 빌드하고 실행하기 위한 컨테이너 도구입니다.
- 컴파일 경고: `-Wall -Wextra -Werror` 기준에서 warning 없이 빌드되어야 합니다.

## 품질

- 단위 테스트를 통해 함수를 검증합니다.
- API 서버를 준비하고 해당 서버에서 기능 테스트를 통해 API가 제대로 동작하는지 확인합니다.
- 엣지 케이스를 최대한 고려합니다.
- 이력서와 포트폴리오에 포함할 수 있을 만큼 완성도 높은 수준으로 구현합니다.
