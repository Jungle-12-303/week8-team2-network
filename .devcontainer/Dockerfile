FROM debian:bookworm

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        curl \
        git \
        ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspaces/week8-team2-network

CMD ["sleep", "infinity"]
