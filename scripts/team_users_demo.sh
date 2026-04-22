#!/usr/bin/env sh
set -eu

PORT="${1:-8080}"
BASE_URL="http://localhost:${PORT}/query"

pretty_print() {
    printf '%s\n' "$1" | sed \
        -e 's/{/{\n  /g' \
        -e 's/}/\n}/g' \
        -e 's/,/,\n  /g'
}

send_query() {
    sql=$1

    printf '%s\n' '========================================'
    printf '$ curl -X POST "%s" \\\n' "$BASE_URL"
    printf '    -H "Content-Type: text/plain" \\\n'
    printf '    --data "%s"\n' "$sql"
    printf '%s\n' '----------------------------------------'

    response="$(curl -sS -X POST "$BASE_URL" \
        -H "Content-Type: text/plain" \
        --data "$sql")"

    pretty_print "$response"
    printf '\n'
}

send_query "INSERT INTO users VALUES ('임정찬', 24);"
send_query "INSERT INTO users VALUES ('주호석', 25);"
send_query "INSERT INTO users VALUES ('이혜연', 23);"
send_query "INSERT INTO users VALUES ('최영빈', 24);"

send_query "SELECT * FROM users;"
send_query "SELECT * FROM users WHERE name = '주호석';"
