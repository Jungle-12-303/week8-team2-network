#!/usr/bin/env sh
set -eu

PORT="${1:-18081}"
TMP_DIR="${TMPDIR:-/tmp}/rwlock_quick_demo.$$"

cleanup() {
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
    if curl -sS -X POST "http://localhost:${PORT}/query" \
        -H "Content-Type: text/plain" \
        --data-raw "SELECT * FROM users WHERE id = 1;" >/dev/null 2>&1; then
        return 0
    fi

    fail "Server is not ready on port ${PORT}"
}

trap cleanup EXIT INT TERM

mkdir -p "$TMP_DIR"

wait_for_server

for i in 1 2; do
    select_file="$TMP_DIR/select-$i.txt"
    insert_file="$TMP_DIR/insert-$i.txt"

    curl -sS -X POST "http://localhost:${PORT}/query" \
        -H "Content-Type: text/plain" \
        --data-raw "SELECT * FROM users WHERE id = 1;" >"$select_file" &

    curl -sS -X POST "http://localhost:${PORT}/query" \
        -H "Content-Type: text/plain" \
        --data-raw "INSERT INTO users VALUES ('Quick', ${i});" >"$insert_file" &
done

wait

for file in "$TMP_DIR"/*.txt; do
    assert_ok "$(cat "$file")"
done

final_response="$(curl -sS -X POST "http://localhost:${PORT}/query" \
    -H "Content-Type: text/plain" \
    --data-raw "SELECT * FROM users;")"
printf '%s' "$final_response" | grep -Fq '"row_count":3' || {
    printf 'Unexpected final response: %s\n' "$final_response" >&2
    exit 1
}

printf '%s\n' "rwlock quick demo test passed."
