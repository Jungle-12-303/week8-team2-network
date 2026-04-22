#!/usr/bin/env sh
set -eu

PORT="${1:-18080}"
SERVER_PID=""
SERVER_LOG="${TMPDIR:-/tmp}/demo_scenario_server_$$.log"

cleanup() {
    if [ -n "$SERVER_PID" ] && kill -0 "$SERVER_PID" 2>/dev/null; then
        kill "$SERVER_PID" 2>/dev/null || true
        wait "$SERVER_PID" 2>/dev/null || true
    fi
    rm -f "$SERVER_LOG"
}

wait_for_server() {
    tries=0
    while [ "$tries" -lt 60 ]; do
        if curl -sS -X POST "http://localhost:${PORT}/query" \
            -H "Content-Type: text/plain" \
            --data-raw "SELECT * FROM users;" >/dev/null 2>&1; then
            return 0
        fi
        tries=$((tries + 1))
        sleep 0.1
    done
    return 1
}

trap cleanup EXIT INT TERM

cd /app

printf '==========================================\n'
printf '[1/3] Internal DB engine and API server link\n'
printf '==========================================\n'

make db_server >/dev/null
./db_server "$PORT" 4 16 32 >"$SERVER_LOG" 2>&1 &
SERVER_PID=$!

wait_for_server || {
    printf 'Failed to start server on port %s\n' "$PORT" >&2
    exit 1
}

sh scripts/tests/http/manual-query.sh "$PORT" <<'EOF'
INSERT INTO users VALUES ('Alice', 20);
SELECT * FROM users WHERE id = 1;
EOF

printf '\n==========================================\n'
printf '[2/3] Multithread concurrency issue\n'
printf '==========================================\n'
sh scripts/rwlock_stress_test.sh

printf '\n==========================================\n'
printf '[3/3] API server architecture\n'
printf '==========================================\n'
sh scripts/multi_client_demo.sh

printf '\n==========================================\n'
printf 'Demo finished successfully\n'
printf '==========================================\n'
