#!/usr/bin/env sh
set -eu

export DEBIAN_FRONTEND=noninteractive

apt-get update >/dev/null
apt-get install -y --no-install-recommends curl python3 >/dev/null

cd /app

MODE="${1:-all}"

run_test() {
    name=$1
    shift
    printf '\n===== RUN %s =====\n' "$name"
    if "$@"; then
        printf '===== PASS %s =====\n' "$name"
    else
        ec=$?
        printf '===== FAIL %s (exit %s) =====\n' "$name" "$ec"
    fi
}

if [ "$MODE" = "all" ] || [ "$MODE" = "basic" ]; then
    run_test "sql/unit-tests.sh" sh scripts/tests/sql/unit-tests.sh
    run_test "http/smoke-test.sh" sh scripts/tests/http/smoke-test.sh 8080
    run_test "http/integration-test.sh" sh scripts/tests/http/integration-test.sh
    run_test "http/protocol-edge-cases.sh" sh scripts/tests/http/protocol-edge-cases.sh
    run_test "http/timeout-test.sh" sh scripts/tests/http/timeout-test.sh
    run_test "concurrency/bucket-lock-stress-test.sh" sh scripts/tests/concurrency/bucket-lock-stress-test.sh
fi

if [ "$MODE" = "all" ] || [ "$MODE" = "ops" ]; then
    run_test "backlog_test.sh" sh scripts/backlog_test.sh
    run_test "queue_503_test.sh" sh scripts/queue_503_test.sh
    run_test "multi_client_demo.sh" sh scripts/multi_client_demo.sh
fi

if [ "$MODE" = "all" ] || [ "$MODE" = "manual" ]; then
    printf '\n===== RUN http/manual-query.sh =====\n'
    PORT=8080
    SERVER_LOG="${TMPDIR:-/tmp}/manual_query_server_$$.log"
    make db_server >/dev/null
    ./db_server "$PORT" 4 16 32 >"$SERVER_LOG" 2>&1 &
    SERVER_PID=$!
    cleanup() {
        if kill -0 "$SERVER_PID" 2>/dev/null; then
            kill "$SERVER_PID" 2>/dev/null || true
            wait "$SERVER_PID" 2>/dev/null || true
        fi
        rm -f "$SERVER_LOG"
    }
    trap cleanup EXIT INT TERM

    tries=0
    while [ "$tries" -lt 60 ]; do
        if sh scripts/tests/http/manual-query.sh "$PORT" "SELECT * FROM users;" >/dev/null 2>&1; then
            break
        fi
        tries=$((tries + 1))
        sleep 0.1
    done

    printf 'Manual query demo output:\n'
    printf '%s\n' "INSERT INTO users VALUES ('Alice', 20);" "SELECT * FROM users WHERE id = 1;" | sh scripts/tests/http/manual-query.sh "$PORT"
    printf '===== PASS http/manual-query.sh =====\n'
fi
