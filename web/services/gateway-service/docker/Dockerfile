# Multi-stage build for Gateway Service

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
COPY services/gateway-service /build/services/gateway-service

# Build gateway-service
WORKDIR /build/services/gateway-service
RUN mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Release .. && make -j$(nproc)

# Stage 2: Runtime
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libssl3 \
    libcurl4 \
    wget \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /build/services/gateway-service/build/gateway_service /app/

RUN mkdir -p /app/logs
RUN chmod +x /app/gateway_service

EXPOSE 8080

ENV CORE_SERVICE_URL=http://core-service:8081
ENV AUTH_SERVICE_URL=http://auth-service:8083
ENV PORT=8080

HEALTHCHECK --interval=30s --timeout=3s --start-period=40s --retries=3 \
    CMD wget --spider -q http://localhost:8080/api/v2/health || exit 1

CMD ["/app/gateway_service"]
