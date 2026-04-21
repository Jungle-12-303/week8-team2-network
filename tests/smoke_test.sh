#!/usr/bin/env bash

set -euo pipefail

PORT="${PORT:-18081}"
SERVER="./mini_dbms_server"

cleanup() {
    if [[ -n "${SERVER_PID:-}" ]]; then
        kill "${SERVER_PID}" >/dev/null 2>&1 || true
        wait "${SERVER_PID}" >/dev/null 2>&1 || true
    fi
}

trap cleanup EXIT

"${SERVER}" "${PORT}" >/tmp/mini_dbms_server.log 2>/tmp/mini_dbms_server.err &
SERVER_PID=$!

sleep 1

curl -sS "http://127.0.0.1:${PORT}/health" | grep -q '"ok":true'
curl -sS -X POST "http://127.0.0.1:${PORT}/users" \
    -H 'Content-Type: application/json' \
    -d '{"name":"Alice","age":20}' | grep -q '"message":"created"'
curl -sS "http://127.0.0.1:${PORT}/users" | grep -q '"row_count":1'
curl -sS "http://127.0.0.1:${PORT}/users/1" | grep -q '"id":1'

echo "smoke test passed"
