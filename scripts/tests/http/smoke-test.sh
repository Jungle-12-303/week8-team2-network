#!/usr/bin/env sh
set -eu

PORT="${1:-8080}"

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

pretty_print() {
    printf '%s\n' "$1" | sed \
        -e 's/{/{\n  /g' \
        -e 's/}/\n}/g' \
        -e 's/,/,\n  /g'
}

insert_response="$(curl -sS -X POST "http://localhost:${PORT}/query" \
  -H "Content-Type: text/plain" \
  --data "INSERT INTO users VALUES ('Bob', 20);")"

printf '%s\n' '================ INSERT ================'
pretty_print "$insert_response"

assert_contains "$insert_response" '"ok":true'
assert_contains "$insert_response" '"action":"insert"'

inserted_id="$(printf '%s' "$insert_response" | sed -n 's/.*"inserted_id":\([0-9][0-9]*\).*/\1/p')"
[ -n "$inserted_id" ] || fail "Could not extract inserted_id from insert response"

select_response="$(curl -sS -X POST "http://localhost:${PORT}/query" \
  -H "Content-Type: text/plain" \
  --data "SELECT * FROM users WHERE id = ${inserted_id};")"

printf '%s\n' '================ SELECT ================'
pretty_print "$select_response"

assert_contains "$select_response" '"ok":true'
assert_contains "$select_response" '"action":"select"'
assert_contains "$select_response" '"Bob"'
assert_contains "$select_response" "\"id\":${inserted_id}"

printf '%s\n' "Smoke test passed."
