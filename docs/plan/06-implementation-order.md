# 06. Implementation Order

## 코드 주석 원칙

각 구현 단계에서 새로 작성하는 코드는 한국어 주석을 친절하게 남기는 것을 완료 조건에 포함합니다. 특히 처음 붙이는 서버 골격, HTTP 파서, SQL 연결, 스레드 풀 도입 단계는 나중에 발표자가 읽고 흐름을 바로 설명할 수 있어야 합니다.

이 문서는 실제 바이브코딩 순서를 안내합니다. 각 단계는 이전 단계가 동작하는지 확인한 뒤 넘어갑니다.

## 1. 빌드 구조 확인

- 기존 `sql_processor`의 `make`, `unit_test`, `perf_test`가 깨지지 않는지 확인합니다.
- 서버 코드를 같은 디렉터리에 둘지, 상위 디렉터리에 둘지 결정합니다.
- MVP는 기존 엔진과 링크하기 쉬운 위치에 서버 target을 추가합니다.

## 2. 단일 연결 서버 만들기

- listening socket을 열고 port를 받습니다.
- client 하나를 accept합니다.
- 요청 내용을 읽고 고정 응답을 반환합니다.
- 이 단계에서는 SQL 엔진과 thread pool을 붙이지 않습니다.

## 3. HTTP 요청 파싱 붙이기

- request line에서 method와 path를 읽습니다.
- header를 빈 줄까지 읽습니다.
- `Content-Length`를 확인해 body를 읽습니다.
- `POST /query` 외 요청은 적절한 HTTP 오류로 응답합니다.
- body는 한 번의 recv로 끝나지 않을 수 있으므로 누적 읽기를 전제로 합니다.
- 잘못 끊긴 요청과 잘못된 길이 값은 같은 계열의 400 오류로 묶습니다.

## 4. SQL 엔진 연결

- request body를 SQL 문자열로 사용합니다.
- `Table *shared_table`을 서버 시작 시 생성합니다.
- `sql_execute(shared_table, body)`를 호출합니다.
- `SQLResult`를 JSON 응답으로 변환합니다.

## 5. 응답 포맷 완성

- INSERT 결과 JSON을 만듭니다.
- SELECT rows JSON을 만듭니다.
- SQL 오류 JSON을 만듭니다.
- 모든 HTTP 응답에 `Content-Length`, `Content-Type`, `Connection: close`를 포함합니다.
- JSON 직렬화 실패와 메모리 할당 실패도 별도 오류 경로로 둡니다.

## 6. 스레드 풀 도입

- worker thread 4개를 서버 시작 시 생성합니다.
- accept loop는 client fd를 job queue에 넣습니다.
- worker는 queue에서 fd를 꺼내 기존 request handler를 호출합니다.
- queue full 상황에는 `503 Service Unavailable`을 반환합니다.
- queue full 응답은 accept loop에서 즉시 처리할 수 있어야 합니다.
- worker 종료 조건과 sentinel 처리를 이 단계에서 함께 설계합니다.

## 7. 동시성 보호

- 전역 또는 server context 안에 `pthread_mutex_t db_mutex`를 둡니다.
- `sql_execute()`와 결과 JSON 변환을 같은 DB lock 안에서 수행합니다.
- 응답 문자열을 만든 뒤 lock을 해제하고 socket write를 수행합니다.
- 종료 시에는 새 요청 수락을 중단하고, 대기 중인 작업을 정리합니다.
- deadlock 방지를 위해 lock 획득 순서는 문서로 고정합니다.

## 8. 테스트 보강

- 기존 SQL 처리기 단위 테스트를 유지합니다.
- curl로 `INSERT`, `SELECT`, 문법 오류 요청을 확인합니다.
- 여러 curl 요청을 동시에 보내도 서버가 죽지 않는지 확인합니다.
- queue full, 잘못된 method/path/body 없는 요청을 확인합니다.

## 9. README 데모 정리

- 빌드 명령어를 적습니다.
- 서버 실행 명령어를 적습니다.
- curl 데모를 적습니다.
- 테스트 결과와 동시성 설계 설명을 적습니다.
- 4분 발표 순서에 맞춰 README 섹션을 배치합니다.
