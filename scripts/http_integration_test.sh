#!/usr/bin/env sh
set -eu

IMAGE_NAME="${IMAGE_NAME:-week8-team2-network-http-test}"
HOST_PORT="${HOST_PORT:-18080}"
CONTAINER_ID=""
TMPDIR=""
SERVER_PID=""
TEST_MODE=""

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
REPO_ROOT="$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)"

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

    printf '%s' "$haystack" | grep -Fq "$needle" || fail "Expected response to contain: $needle"
}

request() {
    expected_status=$1
    response_file=$2
    shift 2

    status_code="$(curl -sS -o "$response_file" -w '%{http_code}' "$@")"
    if [ "$status_code" != "$expected_status" ]; then
        printf 'Unexpected HTTP status.\nExpected: %s\nActual:   %s\nResponse:  %s\n' \
            "$expected_status" "$status_code" "$(cat "$response_file")" >&2
        exit 1
    fi
}

wait_for_server() {
    i=0
    response_file="$TMPDIR/ready.txt"

    while [ "$i" -lt 30 ]; do
        status_code="$(curl -sS -o "$response_file" -w '%{http_code}' "http://localhost:${HOST_PORT}/query" -X GET || true)"
        if [ "$status_code" = "405" ]; then
            return 0
        fi
        i=$((i + 1))
        sleep 1
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

TMPDIR="$(mktemp -d)"

cd "$REPO_ROOT"

start_server

wait_for_server

insert_body="$TMPDIR/insert.json"
request 200 "$insert_body" \
    -X POST "http://localhost:${HOST_PORT}/query" \
    -H "Content-Type: text/plain" \
    --data "INSERT INTO users VALUES ('Alice', 20);"
assert_contains "$(cat "$insert_body")" '"ok":true'
assert_contains "$(cat "$insert_body")" '"action":"insert"'
assert_contains "$(cat "$insert_body")" '"inserted_id":1'
assert_contains "$(cat "$insert_body")" '"row_count":1'

select_body="$TMPDIR/select.json"
request 200 "$select_body" \
    -X POST "http://localhost:${HOST_PORT}/query" \
    -H "Content-Type: text/plain" \
    --data "SELECT * FROM users;"
assert_contains "$(cat "$select_body")" '"ok":true'
assert_contains "$(cat "$select_body")" '"action":"select"'
assert_contains "$(cat "$select_body")" '"row_count":1'
assert_contains "$(cat "$select_body")" '"Alice"'

method_body="$TMPDIR/method.json"
request 405 "$method_body" \
    -X GET "http://localhost:${HOST_PORT}/query"
assert_contains "$(cat "$method_body")" '"ok":false'
assert_contains "$(cat "$method_body")" '"status":"method_not_allowed"'

route_body="$TMPDIR/route.json"
request 404 "$route_body" \
    -X POST "http://localhost:${HOST_PORT}/unknown" \
    -H "Content-Type: text/plain" \
    --data "SELECT * FROM users;"
assert_contains "$(cat "$route_body")" '"ok":false'
assert_contains "$(cat "$route_body")" '"status":"not_found"'

large_body="$TMPDIR/large.json"
large_payload="$(printf '%*s' 5000 '' | tr ' ' 'X')"
request 413 "$large_body" \
    -X POST "http://localhost:${HOST_PORT}/query" \
    -H "Content-Type: text/plain" \
    --data "$large_payload"
assert_contains "$(cat "$large_body")" '"ok":false'
assert_contains "$(cat "$large_body")" '"status":"payload_too_large"'

printf '%s\n' "HTTP integration tests passed."
