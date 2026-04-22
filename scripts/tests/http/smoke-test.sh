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

http_post() {
    path=$1
    body=$2

    if command -v curl >/dev/null 2>&1; then
        curl -sS -X POST "http://localhost:${PORT}${path}" \
            -H "Content-Type: text/plain" \
            --data "$body"
        return
    fi

    if command -v python3 >/dev/null 2>&1; then
        python3 - "$PORT" "$path" "$body" <<'PY'
import http.client
import sys

port = int(sys.argv[1])
path = sys.argv[2]
body = sys.argv[3].encode("utf-8")

conn = http.client.HTTPConnection("localhost", port, timeout=10)
conn.request("POST", path, body=body, headers={"Content-Type": "text/plain"})
response = conn.getresponse()
payload = response.read().decode("utf-8", errors="replace")

if response.status != 200:
    print(payload, end="")
    sys.exit(1)

print(payload, end="")
PY
        return
    fi

    fail "curl or python3 is required"
}

insert_response="$(http_post /query "INSERT INTO users VALUES ('Bob', 20);")"

printf '%s\n' '================ INSERT ================'
pretty_print "$insert_response"

assert_contains "$insert_response" '"ok":true'
assert_contains "$insert_response" '"action":"insert"'

inserted_id="$(printf '%s' "$insert_response" | sed -n 's/.*"inserted_id":\([0-9][0-9]*\).*/\1/p')"
[ -n "$inserted_id" ] || fail "Could not extract inserted_id from insert response"

select_response="$(http_post /query "SELECT * FROM users WHERE id = ${inserted_id};")"

printf '%s\n' '================ SELECT ================'
pretty_print "$select_response"

assert_contains "$select_response" '"ok":true'
assert_contains "$select_response" '"action":"select"'
assert_contains "$select_response" '"Bob"'
assert_contains "$select_response" "\"id\":${inserted_id}"

printf '%s\n' "Smoke test passed."
