FROM debian:bookworm AS builder

RUN apt-get update \
    && apt-get install -y --no-install-recommends build-essential \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .
RUN make clean && make db_server

FROM debian:bookworm-slim AS runtime

WORKDIR /app
COPY --from=builder /app/db_server /app/db_server

EXPOSE 8080
ENTRYPOINT ["/app/db_server"]
CMD ["8080"]
