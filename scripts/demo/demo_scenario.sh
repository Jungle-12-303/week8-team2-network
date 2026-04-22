#!/usr/bin/env sh
set -eu

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
REPO_ROOT="$(CDPATH= cd -- "$SCRIPT_DIR/../.." && pwd)"

cd "$REPO_ROOT"
export DOCKER_CONFIG="$REPO_ROOT/.docker"
mkdir -p "$DOCKER_CONFIG"

if ! docker info >/dev/null 2>&1; then
    exec sh scripts/demo/demo_scenario_inner.sh "$@"
fi

if ! docker compose version >/dev/null 2>&1; then
    if ! docker-compose version >/dev/null 2>&1; then
        exec sh scripts/demo/demo_scenario_inner.sh "$@"
    fi
fi

COMPOSE_CMD="docker compose"
if ! docker compose version >/dev/null 2>&1; then
    COMPOSE_CMD="docker-compose"
fi

exec $COMPOSE_CMD run --rm --build demo sh scripts/demo/demo_scenario_inner.sh "$@"
