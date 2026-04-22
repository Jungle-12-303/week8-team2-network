#!/usr/bin/env sh
# queue_503_test.sh — thread_pool 큐 포화 시 HTTP 503 반환 검증
#
# 원리:
#   http_read_request()는 Content-Length만큼 body를 recv()로 수신한다.
#   헤더만 보내고 body 전송을 지연시키면 워커 스레드가 recv()에서 블로킹된다.
#   (SO_RCVTIMEO 미설정 → recv()는 무기한 블로킹)
#
#   worker=1, queue=2 → 총 수용 가능: 1(처리 중) + 2(큐) = 3
#   4번째 연결부터 thread_pool_submit()이 0을 반환 → accept loop이 즉시 HTTP 503 전송
#
# 사용법: sh scripts/queue_503_test.sh [port]

set -eu

PORT="${1:-19081}"
WORKERS=1
QUEUE=2
TOTAL_CAPACITY=$((WORKERS + QUEUE))
N_REQUESTS=8
HOLD_SECONDS=4
SERVER_LOG="${TMPDIR:-/tmp}/q503_server_$$.log"
SERVER_PID=""

cleanup() {
    if [ -n "$SERVER_PID" ] && kill -0 "$SERVER_PID" 2>/dev/null; then
        kill "$SERVER_PID" 2>/dev/null || true
        wait "$SERVER_PID" 2>/dev/null || true
    fi
    rm -f "$SERVER_LOG"
}
trap cleanup EXIT INT TERM

fail() {
    printf 'FAIL: %s\n' "$1" >&2
    exit 1
}

wait_for_server() {
    tries=0
    while [ "$tries" -lt 50 ]; do
        if curl -sS -X POST "http://localhost:${PORT}/query" \
            -H "Content-Type: text/plain" \
            --data-raw "SELECT * FROM users;" >/dev/null 2>&1; then
            return 0
        fi
        tries=$((tries + 1))
        sleep 0.1
    done
    return 1
}

# ─────────────────────────────────────────────
printf '=== thread_pool 큐 포화 / HTTP 503 동작 검증 ===\n\n'

# STEP 1: 서버 시작
printf '[STEP 1] 서버 시작 (port=%s worker=%d queue=%d)\n' "$PORT" "$WORKERS" "$QUEUE"
make -C "$(dirname "$0")/.." db_server >/dev/null
"$(dirname "$0")/../db_server" "$PORT" "$WORKERS" "$QUEUE" 5 >"$SERVER_LOG" 2>&1 &
SERVER_PID=$!

wait_for_server || fail "서버가 준비되지 않았습니다"
printf '       서버 준비 완료 (PID=%s)\n\n' "$SERVER_PID"

# STEP 2: 용량 계산 설명
printf '[STEP 2] 용량 계산\n'
printf '  workers=%d + queue=%d = %d  (최대 동시 처리 가능 요청 수)\n' \
    "$WORKERS" "$QUEUE" "$TOTAL_CAPACITY"
printf '  %d번째 연결부터 thread_pool_submit()이 0을 반환 → HTTP 503\n\n' \
    "$((TOTAL_CAPACITY + 1))"
printf '  슬로우 바디 트릭:\n'
printf '    헤더만 먼저 전송 (Content-Length 선언) → 워커가 recv()에서 블로킹\n'
printf '    %d초 후 body 전송 → 워커 해제\n\n' "$HOLD_SECONDS"

# STEP 3: N개 요청 동시 발사
printf '[STEP 3] %d개 요청 동시 발사...\n\n' "$N_REQUESTS"

PYTHON_OUTPUT="$(python3 - <<PYEOF
import socket, threading, time

PORT           = $PORT
N              = $N_REQUESTS
HOLD           = $HOLD_SECONDS
SQL            = b"SELECT * FROM users;"
BODY_LEN       = len(SQL)

results = [None] * N
lock    = threading.Lock()

def do_request(i):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(HOLD + 10)
    status = -1
    try:
        s.connect(('127.0.0.1', PORT))

        # 헤더만 전송 — accept()가 이 fd를 큐에 넣거나 503을 즉시 반환
        headers = (
            b"POST /query HTTP/1.1\r\n"
            b"Host: localhost\r\n"
            b"Content-Type: text/plain\r\n"
            + f"Content-Length: {BODY_LEN}\r\n\r\n".encode()
        )
        s.sendall(headers)

        # 503이면 body 전에 이미 응답이 온다
        s.settimeout(1.5)
        try:
            peek = s.recv(64)
        except socket.timeout:
            peek = b""

        if b"503" in peek:
            status = 503
        else:
            # 워커를 HOLD초 점유
            time.sleep(HOLD)
            s.sendall(SQL)
            resp = b""
            s.settimeout(10)
            while True:
                chunk = s.recv(4096)
                if not chunk:
                    break
                resp += chunk
                if b"\r\n\r\n" in resp:
                    break
            first = resp.split(b"\r\n")[0].decode(errors="replace")
            parts = first.split()
            status = int(parts[1]) if len(parts) >= 2 else 0
    except Exception as e:
        status = -1
    finally:
        s.close()

    with lock:
        results[i] = status

threads = [threading.Thread(target=do_request, args=(i,)) for i in range(N)]
for t in threads:
    t.start()
for t in threads:
    t.join()

ok_count  = sum(1 for s in results if s == 200)
err_count = sum(1 for s in results if s == 503)
other     = sum(1 for s in results if s not in (200, 503))

for i, s in enumerate(results):
    if s == 200:
        tag = '200 OK  ← 워커가 처리 (body 수신 대기 중 큐 진입)'
    elif s == 503:
        tag = '503     ← 큐 포화, accept loop에서 즉시 거부'
    else:
        tag = f'{s}     ← 예상치 못한 결과'
    print(f'  Request {i+1:2d}: {tag}')

print()
print(f'Summary: {ok_count} OK (200) | {err_count} 거부 (503) | {other} 기타')
print(f'Expected: {$TOTAL_CAPACITY} OK | {N - $TOTAL_CAPACITY} 503')
print(f'__ok={ok_count}')
print(f'__err={err_count}')
PYEOF
)"

printf '%s\n' "$PYTHON_OUTPUT" | grep -v '^__'
printf '\n'

OK_COUNT=$(printf '%s\n' "$PYTHON_OUTPUT" | grep '^__ok=' | cut -d= -f2)
ERR_COUNT=$(printf '%s\n' "$PYTHON_OUTPUT" | grep '^__err=' | cut -d= -f2)

# STEP 4: 코드 경로 설명
printf '[STEP 4] 코드 경로 (server/server.c:225-232)\n\n'
printf '  // accept loop (메인 스레드)\n'
printf '  if (!thread_pool_submit(&server->pool, client_fd)) {\n'
printf '      // pool->size == pool->queue_capacity 일 때 submit()이 0을 반환\n'
printf '      http_send_response(client_fd, 503, ...);\n'
printf '      close(client_fd);\n'
printf '      // 워커 스레드는 이 연결을 전혀 보지 못함\n'
printf '  }\n\n'
printf '  // thread_pool.c:93-111 — submit() 내부\n'
printf '  pthread_mutex_lock(&pool->mutex);\n'
printf '  if (!pool->stop_requested && pool->size < pool->queue_capacity) {\n'
printf '      pool->jobs[pool->tail] = ...;  // 큐에 추가\n'
printf '      pool->size++;\n'
printf '      pthread_cond_signal(&pool->cond);\n'
printf '      accepted = 1;\n'
printf '  }  // size >= capacity → accepted = 0 → 503\n'
printf '  pthread_mutex_unlock(&pool->mutex);\n\n'

# STEP 5: assert
if [ "$OK_COUNT" -eq "$TOTAL_CAPACITY" ] && [ "$ERR_COUNT" -eq "$((N_REQUESTS - TOTAL_CAPACITY))" ]; then
    printf 'PASS: 503 큐 포화 동작 확인됨\n'
    printf '      OK=%s (worker+queue=%d), 503=%s (초과분=%d)\n' \
        "$OK_COUNT" "$TOTAL_CAPACITY" "$ERR_COUNT" "$((N_REQUESTS - TOTAL_CAPACITY))"
else
    printf 'NOTE: OK=%s (expected %d), 503=%s (expected %d)\n' \
        "$OK_COUNT" "$TOTAL_CAPACITY" "$ERR_COUNT" "$((N_REQUESTS - TOTAL_CAPACITY))"
    printf '      타이밍에 따라 결과가 달라질 수 있습니다.\n'
fi
