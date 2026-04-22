#!/usr/bin/env bash
set -euo pipefail
trap '' PIPE

PORT="${1:-19083}"
WORKERS=4
QUEUE=16
TOTAL_CAPACITY=$((WORKERS + QUEUE))
N_CLIENTS=25
HOLD_SECONDS=5
SERVER_LOG="${TMPDIR:-/tmp}/demo_server_$$.log"
SERVER_PID=""
TMP_DIR="${TMPDIR:-/tmp}/multi_client_demo.$$"

cleanup() {
    if [ -n "${SERVER_PID}" ] && kill -0 "${SERVER_PID}" 2>/dev/null; then
        kill "${SERVER_PID}" 2>/dev/null || true
        wait "${SERVER_PID}" 2>/dev/null || true
    fi
    rm -rf "$TMP_DIR"
    rm -f "$SERVER_LOG"
}
trap cleanup EXIT INT TERM

wait_for_server() {
    tries=0
    while [ "$tries" -lt 60 ]; do
        if curl -sS -X POST "http://localhost:${PORT}/query" \
            -H "Content-Type: text/plain" \
            --data-raw "SELECT * FROM users WHERE id = 1;" >/dev/null 2>&1; then
            return 0
        fi
        tries=$((tries + 1))
        sleep 0.1
    done
    return 1
}

start_server() {
    make -C "$(dirname "$0")/.." db_server >/dev/null
    pkill -f "db_server ${PORT}" 2>/dev/null || true
    sleep 0.2

    export DEBUG_QUEUE=1
    "$(dirname "$0")/../db_server" "$PORT" "$WORKERS" "$QUEUE" \
        >"$SERVER_LOG" 2>&1 &
    SERVER_PID=$!
}

client_request() {
    local index="$1"
    local output_file="$2"
    local sql="INSERT INTO users VALUES ('User', 20);"
    local fd=""
    local probe=""
    local first_line=""
    local status="0"

    exec {fd}<>"/dev/tcp/127.0.0.1/$PORT"
    printf 'POST /query HTTP/1.1\r\nHost: localhost\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n' "${#sql}" >&"${fd}"

    if IFS= read -r -t 0.3 -u "${fd}" probe; then
        if [[ "$probe" == *"503"* ]]; then
            printf '503 큐 포화\n' >"$output_file"
            exec {fd}>&-
            return 0
        fi
    fi

    sleep "$HOLD_SECONDS"
    if ! printf '%s' "$sql" >&"${fd}"; then
        printf '503 큐 포화\n' >"$output_file"
        exec {fd}>&-
        return 0
    fi

    if IFS= read -r -u "${fd}" first_line; then
        status=${first_line#* }
        status=${status%% *}
    fi

    if [ "$status" = "200" ]; then
        printf '200 OK\n' >"$output_file"
    else
        printf '%s\n' "$status" >"$output_file"
    fi

    exec {fd}>&-
}

mkdir -p "$TMP_DIR"

printf '=== 다중 클라이언트 동작 시연 ===\n\n'
printf '설정: workers=%d  queue=%d  total_capacity=%d\n' \
    "$WORKERS" "$QUEUE" "$TOTAL_CAPACITY"
printf '클라이언트: %d개 동시 요청 → %d개 OK 예상, %d개 503 예상\n\n' \
    "$N_CLIENTS" "$TOTAL_CAPACITY" "$((N_CLIENTS - TOTAL_CAPACITY))"

printf '[STEP 1] DEBUG_QUEUE=1 로 서버 시작 (port=%s workers=%d queue=%d)\n' \
    "$PORT" "$WORKERS" "$QUEUE"
start_server
wait_for_server || { printf '실패: 서버가 준비되지 않았습니다\n' >&2; exit 1; }
printf '       서버 준비 완료 (PID=%s)\n' "$SERVER_PID"
printf '       [QUEUE] 로그는 아래 "서버 로그" 섹션에 출력됩니다\n\n'

printf '[STEP 2] %d개 클라이언트 동시 발사 (각 %d초간 워커 점유)\n\n' \
    "$N_CLIENTS" "$HOLD_SECONDS"

declare -a pids=()
for i in $(seq 1 "$N_CLIENTS"); do
    out_file="$TMP_DIR/client-$(printf '%02d' "$i").txt"
    client_request "$i" "$out_file" &
    pids+=("$!")
done

for pid in "${pids[@]}"; do
    wait "$pid"
done

ok_count=0
err_count=0
other=0

for file in "$TMP_DIR"/client-*.txt; do
    line="$(cat "$file")"
    index="${file##*/client-}"
    index="${index%.txt}"
    printf '  클라이언트 %s: %s\n' "$index" "$line"
    case "$line" in
        "200 OK")
            ok_count=$((ok_count + 1))
            ;;
        "503 큐 포화")
            err_count=$((err_count + 1))
            ;;
        *)
            other=$((other + 1))
            ;;
    esac
done

printf '\n요약: %d개 OK | %d개 503 | %d개 기타\n' "$ok_count" "$err_count" "$other"

printf '[STEP 3] 서버 [QUEUE] 로그\n\n'
grep 'QUEUE' "$SERVER_LOG" || printf '  (로그 없음)\n'

printf '\n[STEP 4] 결과 요약\n\n'
printf '  workers=%d  queue=%d  → 수용 가능: %d\n' \
    "$WORKERS" "$QUEUE" "$TOTAL_CAPACITY"
printf '  OK(200) : %s / %d\n' "$ok_count" "$TOTAL_CAPACITY"
printf '  503     : %s / %d\n' "$err_count" "$((N_CLIENTS - TOTAL_CAPACITY))"

if [ "$ok_count" -eq "$TOTAL_CAPACITY" ] && [ "$err_count" -eq "$((N_CLIENTS - TOTAL_CAPACITY))" ]; then
    printf '\n통과\n'
else
    printf '\n주의: 타이밍에 따라 결과가 달라질 수 있습니다.\n'
fi
