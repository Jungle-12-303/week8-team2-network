#!/usr/bin/env sh
set -eu

PORT="${1:-18080}"
SERVER_LOG="${2:-/tmp/rwlock_server.log}"
TMP_DIR="${TMPDIR:-/tmp}/rwlock_stress_test.$$"
SERVER_PID=""

cleanup() {
    if [ -n "$SERVER_PID" ] && kill -0 "$SERVER_PID" 2>/dev/null; then
        kill "$SERVER_PID" 2>/dev/null || true
        wait "$SERVER_PID" 2>/dev/null || true
    fi

    rm -rf "$TMP_DIR"
}

fail() {
    printf '%s\n' "$1" >&2
    exit 1
}

assert_ok() {
    response=$1
    printf '%s' "$response" | grep -Fq '"ok":true' || {
        printf 'Unexpected response: %s\n' "$response" >&2
        exit 1
    }
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
        sleep 0.2
    done

    return 1
}

trap cleanup EXIT INT TERM

mkdir -p "$TMP_DIR"

make db_server >/dev/null

./db_server "$PORT" >"$SERVER_LOG" 2>&1 &
SERVER_PID=$!

wait_for_server || fail "Server did not become ready"

seed_response="$(curl -sS -X POST "http://localhost:${PORT}/query" \
    -H "Content-Type: text/plain" \
    --data-raw "INSERT INTO users VALUES ('Seed', 1);")"
assert_ok "$seed_response"

i=1
job_pids=""
while [ "$i" -le 32 ]; do
    reader_response="$TMP_DIR/reader_$i.json"
    writer_response="$TMP_DIR/writer_$i.json"

    curl -sS -X POST "http://localhost:${PORT}/query" \
        -H "Content-Type: text/plain" \
        --data-raw "SELECT * FROM users;" >"$reader_response" &
    job_pids="$job_pids $!"

    curl -sS -X POST "http://localhost:${PORT}/query" \
        -H "Content-Type: text/plain" \
        --data-raw "INSERT INTO users VALUES ('User${i}', $((20 + i)));" >"$writer_response" &
    job_pids="$job_pids $!"

    i=$((i + 1))
done

for pid in $job_pids; do
    wait "$pid"
done

for response_file in "$TMP_DIR"/*.json; do
    [ -f "$response_file" ] || continue
    assert_ok "$(cat "$response_file")"
done

final_response="$(curl -sS -X POST "http://localhost:${PORT}/query" \
    -H "Content-Type: text/plain" \
    --data-raw "SELECT * FROM users;")"

assert_ok "$final_response"

row_count="$(printf '%s' "$final_response" | sed -n 's/.*"row_count":\([0-9][0-9]*\).*/\1/p')"
[ -n "$row_count" ] || fail "Could not extract row_count from final SELECT response"
[ "$row_count" -eq 33 ] || fail "Expected row_count to be 33, got $row_count"

printf '%s\n' "bucket lock stress test passed."
