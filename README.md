# week8-team2-network

In-memory DBMS API server written in C.

This project exposes a single HTTP endpoint, `POST /query`, and forwards SQL statements to the existing `sql_processor` package.

## What is implemented

- HTTP server entrypoint in C
- `POST /query` only
- Raw socket request parsing with `Content-Length`
- Bounded worker thread pool
- SQL execution through `sql_processor`
- JSON response serialization
- Graceful shutdown with a termination flag
- Docker build and run support

## Build

### Local

```bash
make db_server
```

### Docker

```bash
docker build -t week8-team2-network .
```

## Run

### Local

```bash
./db_server 8080
```

### Docker

```bash
docker run --rm -p 8080:8080 week8-team2-network
```

Or:

```bash
docker compose up --build
```

## API

### Request

삽입
```bash
curl -X POST http://localhost:8080/query \
  -H "Content-Type: text/plain" \
  --data "INSERT INTO users VALUES ('Alice', 20);"
```

조회
```bash
curl -v -X POST http://localhost:8080/query -H "Content-Type: text/plain" --data-raw "SELECT * FROM users;"
```

### Supported SQL

- `INSERT INTO users VALUES ('Alice', 20);`
- `SELECT * FROM users;`
- `SELECT * FROM users WHERE id = 1;`
- `SELECT * FROM users WHERE id >= 10;`
- `SELECT * FROM users WHERE name = 'Alice';`
- `SELECT * FROM users WHERE age > 20;`

## Smoke test

After starting the server, run:

```bash
sh scripts/smoke_test.sh
```

## Project docs

- [`docs/plan/00-plan-manager.md`](docs/plan/00-plan-manager.md)
- [`docs/plan/06-implementation-order.md`](docs/plan/06-implementation-order.md)
- [`docs/plan/07-test-and-quality-plan.md`](docs/plan/07-test-and-quality-plan.md)
- [`docs/plan/08-demo-and-readme-plan.md`](docs/plan/08-demo-and-readme-plan.md)
