#!/usr/bin/env sh
set -eu

PORT="${1:-18080}"
SERVER_PID=""
SERVER_LOG="${TMPDIR:-/tmp}/demo_scenario_server_$$.log"
SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
REPO_ROOT="$(CDPATH= cd -- "$SCRIPT_DIR/../.." && pwd)"

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

cd "$REPO_ROOT"

printf '==========================================\n'
printf '[1/3] 내부 DB 엔진과 API 서버 연결\n'
printf '==========================================\n'

make db_server >/dev/null
./db_server "$PORT" 4 16 32 >"$SERVER_LOG" 2>&1 &
SERVER_PID=$!

wait_for_server || {
    printf '서버를 %s 포트에서 시작하지 못했습니다\n' "$PORT" >&2
    exit 1
}

sh scripts/tests/http/manual-query.sh "$PORT" <<'EOF'
INSERT INTO users VALUES ('Alice', 20);
SELECT * FROM users WHERE id = 1;
EOF

printf '\n==========================================\n'
printf '[2/3] RW 락 동시성 스모크 테스트\n'
printf '==========================================\n'
printf '이 단계는 1단계에서 띄운 서버를 그대로 재사용하며, 보통 몇 초 안에 끝납니다.\n'
sh scripts/tests/concurrency/rwlock_quick_demo_test.sh "$PORT"

if [ "${DEMO_INCLUDE_LONG_CONCURRENCY:-0}" = "1" ]; then
    printf '\n[선택] 긴 동시성 스트레스 테스트 (느리므로 부록용)\n'
    sh scripts/rwlock_stress_test.sh 18081
fi

printf '\n==========================================\n'
printf '[3/3] API 서버 아키텍처\n'
printf '==========================================\n'
sh scripts/multi_client_demo.sh 19083

printf '\n==========================================\n'
printf '데모가 정상적으로 완료되었습니다\n'
printf '==========================================\n'
