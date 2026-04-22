#!/usr/bin/env sh
set -eu

PORT="${1:-18080}"
SERVER_LOG="${2:-/tmp/bucket_lock_server.log}"
TMP_DIR="${TMPDIR:-/tmp}/bucket_lock_stress_test.$$"
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

    fail "Server did not become ready on port ${PORT}"
}

make_binary() {
    if [ ! -x ./db_server ]; then
        make db_server
    fi
}

launch_server() {
    make_binary
    ./db_server "$PORT" >"$SERVER_LOG" 2>&1 &
    SERVER_PID=$!
}

trap cleanup EXIT INT TERM

mkdir -p "$TMP_DIR"

launch_server
wait_for_server

seed_response="$(curl -sS -X POST "http://localhost:${PORT}/query" \
    -H "Content-Type: text/plain" \
    --data-raw "INSERT INTO users VALUES ('Seed', 0);")"
assert_ok "$seed_response"

for i in $(seq 1 32); do
    select_file="$TMP_DIR/select-$i.txt"
    insert_file="$TMP_DIR/insert-$i.txt"

    curl -sS -X POST "http://localhost:${PORT}/query" \
        -H "Content-Type: text/plain" \
        --data-raw "SELECT * FROM users;" >"$select_file" &

    curl -sS -X POST "http://localhost:${PORT}/query" \
        -H "Content-Type: text/plain" \
        --data-raw "INSERT INTO users VALUES ('Bucket', ${i});" >"$insert_file" &
done

wait

for file in "$TMP_DIR"/*.txt; do
    assert_ok "$(cat "$file")"
done

final_response="$(curl -sS -X POST "http://localhost:${PORT}/query" \
    -H "Content-Type: text/plain" \
    --data-raw "SELECT * FROM users;")"
printf '%s' "$final_response" | grep -Fq '"row_count":33' || {
    printf 'Unexpected final response: %s\n' "$final_response" >&2
    exit 1
}

printf '%s\n' "bucket lock stress test passed."
