#!/usr/bin/env sh
set -eu

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"

if command -v bash >/dev/null 2>&1; then
    exec bash "$SCRIPT_DIR/multi_client_demo_inner.sh" "$@"
fi

printf '%s\n' 'bash 실행기가 필요합니다.' >&2
exit 1
