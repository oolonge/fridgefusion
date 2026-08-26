# Multi-stage build for Core Service

# Stage 1: Builder
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libssl-dev \
    libboost-dev \
    libasio-dev \
    libcurl4-openssl-dev \
    wget \
    git \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build

# Copy source code
COPY third_party /build/third_party
COPY services/core-service /build/services/core-service

# Build core-service
WORKDIR /build/services/core-service
RUN mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Release .. && make -j$(nproc)

# Stage 2: Runtime
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libssl3 \
    libcurl4 \
    wget \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /build/services/core-service/build/core_service /app/

RUN mkdir -p /app/logs
RUN chmod +x /app/core_service

EXPOSE 8081

ENV DATA_SERVICE_URL=http://data-service:8082
ENV PORT=8081

HEALTHCHECK --interval=30s --timeout=3s --start-period=40s --retries=3 \
    CMD wget --spider -q http://localhost:8081/health || exit 1

CMD ["/app/core_service"]
