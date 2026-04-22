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
        printf '예상과 다른 응답입니다: %s\n' "$response" >&2
        exit 1
    }
}

wait_for_server() {
    if curl -sS -X POST "http://localhost:${PORT}/query" \
        -H "Content-Type: text/plain" \
        --data-raw "SELECT * FROM users WHERE id = 1;" >/dev/null 2>&1; then
        return 0
    fi

    fail "서버가 ${PORT} 포트에서 준비되지 않았습니다"
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
    printf '최종 응답이 예상과 다릅니다: %s\n' "$final_response" >&2
    exit 1
}

printf '%s\n' "RW 락 빠른 데모 테스트가 통과했습니다."
