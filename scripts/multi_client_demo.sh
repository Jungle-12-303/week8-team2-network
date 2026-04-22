#!/usr/bin/env sh
# multi_client_demo.sh — 실제 서버 설정으로 다중 클라이언트 동작 시연
#
# 서버를 DEBUG_QUEUE=1 로 시작하고 25개 클라이언트를 동시에 보내
# 큐에 쌓이는 과정과 503 반환 순간을 실시간으로 확인한다.
#
# 사용법: sh scripts/multi_client_demo.sh [port]

set -eu

PORT="${1:-19083}"
WORKERS=4
QUEUE=16
TOTAL_CAPACITY=$((WORKERS + QUEUE))   # 20
N_CLIENTS=25                          # 20은 처리, 5는 503
HOLD_SECONDS=5
SERVER_LOG="${TMPDIR:-/tmp}/demo_server_$$.log"
SERVER_PID=""

cleanup() {
    if [ -n "$SERVER_PID" ] && kill -0 "$SERVER_PID" 2>/dev/null; then
        kill "$SERVER_PID" 2>/dev/null || true
        wait "$SERVER_PID" 2>/dev/null || true
    fi
    rm -f "$SERVER_LOG"
}
trap cleanup EXIT INT TERM

wait_for_server() {
    tries=0
    while [ "$tries" -lt 60 ]; do
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

# ─────────────────────────────────────────────────────────
printf '=== 다중 클라이언트 동작 시연 ===\n\n'
printf '설정: workers=%d  queue=%d  total_capacity=%d\n' \
    "$WORKERS" "$QUEUE" "$TOTAL_CAPACITY"
printf '클라이언트: %d개 동시 요청 → %d개 OK 예상, %d개 503 예상\n\n' \
    "$N_CLIENTS" "$TOTAL_CAPACITY" "$((N_CLIENTS - TOTAL_CAPACITY))"

# STEP 1: 서버 시작 (DEBUG_QUEUE=1, 실제 설정)
printf '[STEP 1] DEBUG_QUEUE=1 로 서버 시작 (port=%s workers=%d queue=%d)\n' \
    "$PORT" "$WORKERS" "$QUEUE"
make -C "$(dirname "$0")/.." db_server >/dev/null
pkill -f "db_server $PORT" 2>/dev/null || true; sleep 0.2

export DEBUG_QUEUE=1
"$(dirname "$0")/../db_server" "$PORT" "$WORKERS" "$QUEUE" \
    >"$SERVER_LOG" 2>&1 &
SERVER_PID=$!

wait_for_server || { printf '실패: 서버가 준비되지 않았습니다\n' >&2; exit 1; }
printf '       서버 준비 완료 (PID=%s)\n' "$SERVER_PID"
printf '       [QUEUE] 로그는 아래 "서버 로그" 섹션에 출력됩니다\n\n'

# STEP 2: 25개 클라이언트 동시 발사
printf '[STEP 2] %d개 클라이언트 동시 발사 (각 %d초간 워커 점유)\n\n' \
    "$N_CLIENTS" "$HOLD_SECONDS"

CLIENT_OUTPUT="$(python3 - <<PYEOF
import socket, threading, time

PORT  = $PORT
N     = $N_CLIENTS
HOLD  = $HOLD_SECONDS
SQL   = b"INSERT INTO users VALUES ('User', 20);"

results = [None] * N
lock    = threading.Lock()
start   = time.time()

def do_request(i):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(HOLD + 10)
    status = -1
    elapsed = 0.0
    try:
        s.connect(('127.0.0.1', PORT))
        headers = (
            b"POST /query HTTP/1.1\r\nHost: localhost\r\n"
            b"Content-Type: text/plain\r\n"
            + f"Content-Length: {len(SQL)}\r\n\r\n".encode()
        )
        s.sendall(headers)

        # 503은 body 전에 즉시 도착
        s.settimeout(1.5)
        try:
            peek = s.recv(64)
        except socket.timeout:
            peek = b""

        if b"503" in peek:
            status = 503
            elapsed = time.time() - start
        else:
            time.sleep(HOLD)
            s.sendall(SQL)
            resp = b""
            s.settimeout(10)
            while True:
                chunk = s.recv(4096)
                if not chunk: break
                resp += chunk
                if b"\r\n\r\n" in resp: break
            line = resp.split(b"\r\n")[0].decode(errors="replace")
            parts = line.split()
            status = int(parts[1]) if len(parts) >= 2 else 0
            elapsed = time.time() - start
    except Exception as e:
        status = -1
        elapsed = time.time() - start
    finally:
        s.close()
    with lock:
        results[i] = (status, elapsed)

threads = [threading.Thread(target=do_request, args=(i,)) for i in range(N)]
for t in threads: t.start()
for t in threads: t.join()

ok_count  = sum(1 for s, _ in results if s == 200)
err_count = sum(1 for s, _ in results if s == 503)
other     = sum(1 for s, _ in results if s not in (200, 503))

for i, (s, t) in enumerate(results):
    if s == 200:
        tag = f'200 OK  ({t:.2f}s)'
    elif s == 503:
        tag = f'503     ({t:.2f}s)  ← 큐 포화'
    else:
        tag = f'{s}     ({t:.2f}s)'
    print(f'  클라이언트 {i+1:2d}: {tag}')

print()
print(f'요약: {ok_count}개 OK | {err_count}개 503 | {other}개 기타')
print(f'__ok={ok_count}')
print(f'__err={err_count}')
PYEOF
)"

printf '%s\n' "$CLIENT_OUTPUT" | grep -v '^__'
printf '\n'

OK_COUNT=$(printf '%s\n' "$CLIENT_OUTPUT" | grep '^__ok=' | cut -d= -f2)
ERR_COUNT=$(printf '%s\n' "$CLIENT_OUTPUT" | grep '^__err=' | cut -d= -f2)

# STEP 3: 서버 로그 출력
printf '[STEP 3] 서버 [QUEUE] 로그\n\n'
grep 'QUEUE' "$SERVER_LOG" || printf '  (로그 없음)\n'

printf '\n'

# STEP 4: 결과 요약
printf '[STEP 4] 결과 요약\n\n'
printf '  workers=%d  queue=%d  → 수용 가능: %d\n' \
    "$WORKERS" "$QUEUE" "$TOTAL_CAPACITY"
printf '  OK(200) : %s / %d\n' "$OK_COUNT" "$TOTAL_CAPACITY"
printf '  503     : %s / %d\n' "$ERR_COUNT" "$((N_CLIENTS - TOTAL_CAPACITY))"

if [ "$OK_COUNT" -eq "$TOTAL_CAPACITY" ] && [ "$ERR_COUNT" -eq "$((N_CLIENTS - TOTAL_CAPACITY))" ]; then
    printf '\n통과\n'
else
    printf '\n주의: 타이밍에 따라 결과가 달라질 수 있습니다.\n'
fi
