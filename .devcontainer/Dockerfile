FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive \
    TZ=Asia/Seoul \
    LANG=ko_KR.UTF-8 \
    LANGUAGE=ko_KR.UTF-8

RUN apt-get update && apt-get install -y --no-install-recommends \
    locales \
    tzdata \
    build-essential \
    bash \
    sudo \
    gdb \
    vim \
    curl \
    git \
    wget \
    python3 \
    cmake \
    make \
    gcc \
    valgrind \
    net-tools \
    iproute2 \
    ca-certificates \
    && locale-gen ko_KR.UTF-8 \
    && update-locale LANG=ko_KR.UTF-8 \
    && rm -rf /var/lib/apt/lists/*

RUN useradd -m -s /bin/bash jungle && usermod -aG sudo jungle
RUN echo "jungle ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/jungle

USER jungle
WORKDIR /workspaces/week8-team2-network
