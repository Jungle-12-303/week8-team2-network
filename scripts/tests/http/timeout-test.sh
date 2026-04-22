#!/usr/bin/env sh
set -eu

IMAGE_NAME="${IMAGE_NAME:-week8-team2-network-http-test}"
HOST_PORT="${HOST_PORT:-18082}"
CONTAINER_ID=""
SERVER_PID=""
TMPDIR=""
PYTHON_BIN=""
TEST_MODE=""

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
REPO_ROOT="$(CDPATH= cd -- "$SCRIPT_DIR/../../.." && pwd)"

cleanup() {
    if [ -n "$CONTAINER_ID" ]; then
        docker stop "$CONTAINER_ID" >/dev/null 2>&1 || true
    fi

    if [ -n "$SERVER_PID" ]; then
        kill "$SERVER_PID" >/dev/null 2>&1 || true
        wait "$SERVER_PID" >/dev/null 2>&1 || true
    fi

    if [ -n "$TMPDIR" ] && [ -d "$TMPDIR" ]; then
        rm -rf "$TMPDIR"
    fi
}

trap cleanup EXIT INT TERM

fail() {
    printf '%s\n' "$1" >&2
    exit 1
}

assert_contains() {
    haystack=$1
    needle=$2

    printf '%s' "$haystack" | grep -Fq "$needle" || {
        printf 'Expected response to contain: %s\nActual response: %s\n' "$needle" "$haystack" >&2
        exit 1
    }
}

wait_for_server() {
    tries=0
    while [ "$tries" -lt 50 ]; do
        status_code="$(curl -sS -o "$TMPDIR/ready.txt" -w '%{http_code}' "http://localhost:${HOST_PORT}/query" -X GET || true)"
        if [ "$status_code" = "405" ]; then
            return 0
        fi
        tries=$((tries + 1))
        sleep 0.2
    done

    fail "Server did not become ready on port ${HOST_PORT}"
}

start_server() {
    if command -v docker >/dev/null 2>&1 && docker info >/dev/null 2>&1; then
        TEST_MODE="docker"
        docker build -t "$IMAGE_NAME" .
        CONTAINER_ID="$(docker run -d --rm -p "${HOST_PORT}:8080" "$IMAGE_NAME")"
        return 0
    fi

    TEST_MODE="local"
    make db_server
    ./db_server "$HOST_PORT" >/dev/null 2>&1 &
    SERVER_PID="$!"
}

if command -v python3 >/dev/null 2>&1; then
    PYTHON_BIN=python3
elif command -v python >/dev/null 2>&1; then
    PYTHON_BIN=python
else
    fail "python3 is required for HTTP timeout tests"
fi

TMPDIR="$(mktemp -d)"
cd "$REPO_ROOT"

start_server
wait_for_server

timeout_result="$TMPDIR/timeout-result.txt"

"$PYTHON_BIN" - "$HOST_PORT" "$timeout_result" <<'PY'
import pathlib
import socket
import sys
import threading
import time

port = int(sys.argv[1])
result_path = pathlib.Path(sys.argv[2])
timeout_seconds = 5
slow_request = (
    b"POST /query HTTP/1.1\r\n"
    b"Host: localhost\r\n"
    b"Content-Type: text/plain\r\n"
    b"Content-Length: 64\r\n"
    b"\r\n"
    b"SELECT * FROM "
)
normal_request = (
    b"POST /query HTTP/1.1\r\n"
    b"Host: localhost\r\n"
    b"Content-Type: text/plain\r\n"
    b"Content-Length: 39\r\n"
    b"\r\n"
    b"INSERT INTO users VALUES ('Timeout', 1);"
)

def read_all(sock):
    chunks = []
    while True:
        data = sock.recv(4096)
        if not data:
            break
        chunks.append(data)
    return b"".join(chunks)

slow_sockets = []
for _ in range(4):
    sock = socket.create_connection(("127.0.0.1", port), timeout=5)
    sock.sendall(slow_request)
    slow_sockets.append(sock)

time.sleep(1)

normal_state = {}

def send_normal_request():
    start = time.monotonic()
    with socket.create_connection(("127.0.0.1", port), timeout=5) as sock:
        sock.sendall(normal_request)
        sock.shutdown(socket.SHUT_WR)
        normal_state["response"] = read_all(sock)
    normal_state["elapsed"] = time.monotonic() - start

normal_thread = threading.Thread(target=send_normal_request)
normal_thread.start()

time.sleep(timeout_seconds + 1)

slow_response = read_all(slow_sockets[0]).decode("utf-8", errors="replace")
if "HTTP/1.1 408 Request Timeout" not in slow_response:
    raise SystemExit(f"slow request did not time out as expected: {slow_response!r}")

for sock in slow_sockets:
    sock.close()

normal_thread.join(timeout=timeout_seconds + 10)
if normal_thread.is_alive():
    raise SystemExit("normal request did not complete after timeout release")

normal_response = normal_state.get("response", b"").decode("utf-8", errors="replace")
elapsed = float(normal_state.get("elapsed", -1))
if "HTTP/1.1 200 OK" not in normal_response:
    raise SystemExit(f"normal request failed: {normal_response!r}")
if '"ok":true' not in normal_response:
    raise SystemExit(f"normal request body missing ok=true: {normal_response!r}")
if elapsed < 2.0:
    raise SystemExit(f"normal request completed too quickly ({elapsed:.2f}s); timeout was not exercised")
if elapsed > timeout_seconds + 10:
    raise SystemExit(f"normal request took too long after timeout ({elapsed:.2f}s)")

result_path.write_text(
    f"slow_timeout=ok\nnormal_elapsed={elapsed:.2f}\nnormal_status=ok\n",
    encoding="utf-8",
)
PY

printf '%s\n' "HTTP timeout test passed."
