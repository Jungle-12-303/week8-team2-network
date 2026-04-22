#!/usr/bin/env sh
set -eu

IMAGE_NAME="${IMAGE_NAME:-week8-team2-network-http-test}"
HOST_PORT="${HOST_PORT:-18081}"
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

send_raw_request() {
    case_name=$1
    response_file=$2

    "$PYTHON_BIN" - "$HOST_PORT" "$case_name" "$response_file" <<'PY'
import pathlib
import socket
import sys

port = int(sys.argv[1])
case_name = sys.argv[2]
response_file = pathlib.Path(sys.argv[3])

requests = {
    "empty_body": b"POST /query HTTP/1.1\r\nHost: localhost\r\nContent-Type: text/plain\r\nContent-Length: 0\r\n\r\n",
    "missing_content_length": b"POST /query HTTP/1.1\r\nHost: localhost\r\nContent-Type: text/plain\r\n\r\n",
    "malformed_request_line": b"POST /query\r\nHost: localhost\r\nContent-Type: text/plain\r\nContent-Length: 0\r\n\r\n",
    "content_length_mismatch": b"POST /query HTTP/1.1\r\nHost: localhost\r\nContent-Type: text/plain\r\nContent-Length: 50\r\n\r\nSELECT * FROM users;",
}

request = requests[case_name]

with socket.create_connection(("127.0.0.1", port), timeout=5) as sock:
    sock.sendall(request)
    sock.shutdown(socket.SHUT_WR)

    chunks = []
    while True:
        data = sock.recv(4096)
        if not data:
            break
        chunks.append(data)

response_file.write_bytes(b"".join(chunks))
PY
}

show_response() {
    title=$1
    response_file=$2

    printf '%s\n' "================ ${title} ================"
    tr -d '\r' < "$response_file"
    printf '\n'
}

run_case() {
    title=$1
    case_name=$2
    response_file=$3
    expected_status=$4
    shift 4

    send_raw_request "$case_name" "$response_file"
    response="$(tr -d '\r' < "$response_file")"
    show_response "$title" "$response_file"

    assert_contains "$response" "$expected_status"

    while [ "$#" -gt 0 ]; do
        assert_contains "$response" "$1"
        shift
    done
}

if ! command -v python3 >/dev/null 2>&1 && ! command -v python >/dev/null 2>&1; then
    fail "python3 is required for HTTP protocol edge-case tests"
fi

if command -v python3 >/dev/null 2>&1; then
    PYTHON_BIN=python3
else
    PYTHON_BIN=python
fi

TMPDIR="$(mktemp -d)"
cd "$REPO_ROOT"

start_server
wait_for_server

run_case \
    "EMPTY BODY" \
    "empty_body" \
    "$TMPDIR/empty_body.txt" \
    "HTTP/1.1 200 OK" \
    '"status":"syntax_error"' \
    '"error_code":1064' \
    '"sql_state":"42000"'

run_case \
    "MISSING CONTENT-LENGTH" \
    "missing_content_length" \
    "$TMPDIR/missing_content_length.txt" \
    "HTTP/1.1 400 Bad Request" \
    '"status":"bad_request"' \
    '"Missing Content-Length"'

run_case \
    "MALFORMED REQUEST LINE" \
    "malformed_request_line" \
    "$TMPDIR/malformed_request_line.txt" \
    "HTTP/1.1 400 Bad Request" \
    '"status":"bad_request"' \
    '"Malformed HTTP request line"'

run_case \
    "CONTENT-LENGTH MISMATCH" \
    "content_length_mismatch" \
    "$TMPDIR/content_length_mismatch.txt" \
    "HTTP/1.1 400 Bad Request" \
    '"status":"bad_request"' \
    '"Incomplete request body"'

printf '%s\n' "HTTP protocol edge-case tests passed."
