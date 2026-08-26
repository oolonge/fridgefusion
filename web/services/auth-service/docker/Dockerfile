# Multi-stage build for Auth Service

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
COPY services/auth-service /build/services/auth-service

# Build auth-service
WORKDIR /build/services/auth-service
RUN mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Release .. && make -j$(nproc)

# Stage 2: Runtime
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libssl3 \
    libcurl4 \
    wget \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /build/services/auth-service/build/auth_service /app/

RUN mkdir -p /app/logs
RUN chmod +x /app/auth_service

EXPOSE 8083

ENV CORE_SERVICE_URL=http://core-service:8081
ENV PORT=8083
ENV JWT_SECRET=your-secret-key-change-in-production
ENV JWT_ISSUER=FridgeFusion
ENV JWT_EXPIRATION_HOURS=24

HEALTHCHECK --interval=30s --timeout=3s --start-period=40s --retries=3 \
    CMD wget --spider -q http://localhost:8083/auth/health || exit 1

CMD ["/app/auth_service"]
