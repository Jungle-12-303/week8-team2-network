#!/usr/bin/env sh
set -eu

PORT="${1:-8080}"

pretty_print() {
    printf '%s\n' "$1" | sed \
        -e 's/{/{\n  /g' \
        -e 's/}/\n}/g' \
        -e 's/,/,\n  /g'
}

send_query() {
    sql=$1

    response="$(curl -sS -X POST "http://localhost:${PORT}/query" \
        -H "Content-Type: text/plain" \
        --data "$sql")"

    printf '%s\n' '========================================'
    printf 'SQL> %s\n' "$sql"
    printf '%s\n' '----------------------------------------'
    pretty_print "$response"
    printf '\n'
}

if [ "$#" -gt 1 ]; then
    shift
    send_query "$*"
    exit 0
fi

if [ "$#" -eq 1 ] && [ -t 0 ]; then
    printf '%s\n' 'Enter one SQL statement per line. Type "exit" to quit.'
    while :; do
        printf '%s' 'SQL> '
        if ! IFS= read -r sql; then
            printf '\n'
            break
        fi
        case "$sql" in
            "" )
                continue
                ;;
            exit|quit)
                break
                ;;
        esac
        send_query "$sql"
    done
    exit 0
fi

if [ "$#" -eq 1 ]; then
    while IFS= read -r sql; do
        case "$sql" in
            "" )
                continue
                ;;
            exit|quit)
                break
                ;;
        esac
        send_query "$sql"
    done
    exit 0
fi

printf '%s\n' 'Usage: sh scripts/manual_query.sh [PORT] [SQL...]' >&2
printf '%s\n' 'Example: sh scripts/manual_query.sh 8080 "INSERT INTO users VALUES (''Alice'', 20);"' >&2
exit 1
