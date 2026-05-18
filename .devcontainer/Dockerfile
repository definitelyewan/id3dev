FROM debian:bookworm-slim

ARG DEBIAN_FRONTEND=noninteractive

# Version must match the CLion Gateway version installed on your host machine.
# Check Help → About in CLion to find the exact version string (e.g. 2026.1.1).
ARG CLION_VERSION=2026.1.1

# Keep the image small but include a complete C workflow: GCC + GNU Make + clang tooling.
RUN apt-get update && apt-get install -y --no-install-recommends \
    bash \
    build-essential \
    ca-certificates \
    ccache \
    clangd \
    clang-tidy \
    cmake \
    curl \
    gdb \
    git \
    make \
    pkg-config \
    python3 \
    bear \
    valgrind \
 && rm -rf /var/lib/apt/lists/*

# Pre-install the CLion remote development backend so CLion Gateway can connect
# offline. The backend must match the CLion version on the host (CLION_VERSION).
# Selects the correct archive for the container's CPU architecture automatically.
RUN set -eux; \
    ARCH="$(uname -m)"; \
    if [ "$ARCH" = "aarch64" ]; then \
        CLION_ARCHIVE="CLion-${CLION_VERSION}-aarch64.tar.gz"; \
    else \
        CLION_ARCHIVE="CLion-${CLION_VERSION}.tar.gz"; \
    fi; \
    mkdir -p /opt/clion; \
    curl -fsSL "https://download.jetbrains.com/cpp/${CLION_ARCHIVE}" \
        | tar -xz --strip-components=1 -C /opt/clion; \
    /opt/clion/bin/remote-dev-server.sh registerBackendLocationForGateway

ENV CC=gcc \
    CXX=g++

WORKDIR /workspaces/id3dev

