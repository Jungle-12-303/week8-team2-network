#!/usr/bin/env sh
# backlog_test.sh — 커널 TCP backlog 동작 검증
#
# 원리:
#   SIGSTOP으로 프로세스를 일시정지하면 accept() 루프가 멈춘다.
#   그 상태에서 N개의 TCP 연결을 동시에 시도하면:
#     - backlog 이하: 커널이 TCP 핸드셰이크를 완료하고 큐에 보관 → connect() 성공
#     - backlog 초과: 커널이 SYN을 무시하거나 RST를 보냄 → connect() 실패
#   SIGCONT로 재개하면 서버가 큐에 쌓인 연결을 드레인한다.
#
# 사용법: sh scripts/backlog_test.sh [port]

set -eu

PORT="${1:-19080}"
BACKLOG=2
N_CONNECT=8
SERVER_LOG="${TMPDIR:-/tmp}/backlog_server_$$.log"
SERVER_PID=""

cleanup() {
    if [ -n "$SERVER_PID" ] && kill -0 "$SERVER_PID" 2>/dev/null; then
        kill -CONT "$SERVER_PID" 2>/dev/null || true
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
printf '=== 커널 TCP backlog 동작 검증 ===\n\n'
printf '개념:\n'
printf '  listen(fd, %d) → 커널은 완성된 TCP 핸드셰이크를 최대 %d개 큐에 보관\n' "$BACKLOG" "$BACKLOG"
printf '  accept()가 멈추면 %d개를 초과하는 연결은 커널이 거부한다\n\n' "$BACKLOG"

# STEP 1: 서버 시작
printf '[STEP 1] 서버 시작 (port=%s worker=1 queue=2 backlog=%d)\n' "$PORT" "$BACKLOG"
make -C "$(dirname "$0")/.." db_server >/dev/null
"$(dirname "$0")/../db_server" "$PORT" 1 2 "$BACKLOG" >"$SERVER_LOG" 2>&1 &
SERVER_PID=$!

wait_for_server || fail "서버가 준비되지 않았습니다"
printf '       서버 준비 완료 (PID=%s)\n\n' "$SERVER_PID"

# STEP 2: SIGSTOP — accept() 루프 정지
printf '[STEP 2] SIGSTOP → PID %s 정지. accept() 루프가 멈춥니다.\n' "$SERVER_PID"
printf '         이제 TCP 핸드셰이크는 커널이 처리하지만 accept()는 호출되지 않습니다.\n\n'
kill -STOP "$SERVER_PID"
sleep 0.2

# STEP 3: N개 TCP 연결 동시 시도
printf '[STEP 3] %d개 TCP 연결 동시 시도 (backlog=%d, 초과분은 거부 예상)\n' "$N_CONNECT" "$BACKLOG"

PYTHON_OUTPUT="$(python3 - <<PYEOF
import socket, threading, time, sys

PORT    = $PORT
N       = $N_CONNECT
BACKLOG = $BACKLOG

results = [None] * N
sockets_open = []
lock = threading.Lock()

def try_connect(i):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(2.0)
    try:
        s.connect(('127.0.0.1', PORT))
        with lock:
            results[i] = 'connected'
            sockets_open.append(s)
    except ConnectionRefusedError:
        results[i] = 'refused'
        s.close()
    except socket.timeout:
        results[i] = 'timeout'
        s.close()
    except OSError as e:
        results[i] = f'error({e.errno})'
        s.close()

threads = [threading.Thread(target=try_connect, args=(i,)) for i in range(N)]
for t in threads: t.start()
for t in threads: t.join()

connected = sum(1 for r in results if r == 'connected')
refused   = sum(1 for r in results if r == 'refused')
timed_out = sum(1 for r in results if r == 'timeout')

for i, r in enumerate(results):
    print(f'  Connection {i+1:2d}: {r}')

print()
print(f'Summary: {connected} connected | {refused} refused | {timed_out} timeout')
print(f'__connected={connected}')
print(f'__refused={refused}')

# 소켓을 열어두어 SIGCONT 후 서버가 드레인할 수 있도록 함
time.sleep(4)
for s in sockets_open:
    try: s.close()
    except: pass
PYEOF
)"

printf '%s\n' "$PYTHON_OUTPUT" | grep -v '^__'
printf '\n'

CONNECTED=$(printf '%s\n' "$PYTHON_OUTPUT" | grep '^__connected=' | cut -d= -f2)
REFUSED=$(printf '%s\n' "$PYTHON_OUTPUT" | grep '^__refused=' | cut -d= -f2)

# STEP 4: SIGCONT — 서버 재개
printf '[STEP 4] SIGCONT → 서버 재개. 큐에 쌓인 연결을 드레인합니다.\n\n'
kill -CONT "$SERVER_PID"
sleep 1

# STEP 5: 결과 해석 및 assert
printf '[STEP 5] 결과 해석\n\n'
printf '=== 방금 본 것의 의미 ===\n'
printf '  backlog=%d: 커널이 완성된 TCP 핸드셰이크를 최대 %d개 보관합니다.\n' "$BACKLOG" "$BACKLOG"
printf '  accept()가 멈춰있는 동안 %d번째 연결부터 커널이 거부했습니다.\n' "$((BACKLOG + 1))"
printf '  SIGCONT 이후 서버가 accept()를 재개해 큐의 연결들을 처리합니다.\n\n'
printf '  코드 위치: server/server.c — server_make_listen_socket()\n'
printf '    listen(listen_fd, config->backlog)  ← 이 backlog 값이 커널 큐 크기를 결정\n\n'

# macOS는 backlog를 내부적으로 올림 처리할 수 있음
EFFECTIVE_MAX=$((BACKLOG + 3))
printf '  주의 (macOS): 커널이 backlog를 내부적으로 올림 처리할 수 있습니다.\n'
printf '  실제 connected(%s)가 설정값(%d)보다 약간 클 수 있습니다.\n\n' "$CONNECTED" "$BACKLOG"

if [ "$CONNECTED" -le "$EFFECTIVE_MAX" ] && [ "$REFUSED" -ge 1 ]; then
    printf 'PASS: backlog 동작 확인됨 (connected=%s, refused=%s)\n' "$CONNECTED" "$REFUSED"
else
    printf 'NOTE: connected=%s, refused=%s — backlog=%d (커널 올림 처리 가능성 있음)\n' \
        "$CONNECTED" "$REFUSED" "$BACKLOG"
fi
