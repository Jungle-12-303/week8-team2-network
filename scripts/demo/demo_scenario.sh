#!/usr/bin/env sh
set -eu

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
REPO_ROOT="$(CDPATH= cd -- "$SCRIPT_DIR/../.." && pwd)"

cd "$REPO_ROOT"
export DOCKER_CONFIG="$REPO_ROOT/.docker"
mkdir -p "$DOCKER_CONFIG"

if ! docker compose version >/dev/null 2>&1; then
    if ! docker-compose version >/dev/null 2>&1; then
        printf '%s\n' 'Docker Compose is required to run this demo.' >&2
        exit 1
    fi
fi

if ! docker info >/dev/null 2>&1; then
    docker desktop start >/dev/null 2>&1 || true
    tries=0
    while [ "$tries" -lt 60 ]; do
        if docker info >/dev/null 2>&1; then
            break
        fi
        tries=$((tries + 1))
        sleep 2
    done
fi

if ! docker info >/dev/null 2>&1; then
    printf '%s\n' 'Docker Desktop or the Docker engine must be running.' >&2
    exit 1
fi

COMPOSE_CMD="docker compose"
if ! docker compose version >/dev/null 2>&1; then
    COMPOSE_CMD="docker-compose"
fi

exec $COMPOSE_CMD run --rm --build demo sh scripts/demo/demo_scenario_inner.sh "$@"
