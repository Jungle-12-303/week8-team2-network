#!/usr/bin/env sh
set -eu

PORT="${1:-8080}"

curl -sS -X POST "http://localhost:${PORT}/query" \
  -H "Content-Type: text/plain" \
  --data "INSERT INTO users VALUES ('Alice', 20);"

curl -sS -X POST "http://localhost:${PORT}/query" \
  -H "Content-Type: text/plain" \
  --data "SELECT * FROM users;"

