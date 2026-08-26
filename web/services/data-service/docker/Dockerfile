# Multi-stage build for Data Service

# Stage 1: Builder
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    qt6-base-dev \
    qt6-base-dev-tools \
    libqt6sql6 \
    libqt6sql6-psql \
    libpq-dev \
    libssl-dev \
    libboost-dev \
    libasio-dev \
    zlib1g-dev \
    wget \
    git \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build

# Copy source code
COPY libs /build/libs
COPY third_party /build/third_party
COPY services/data-service /build/services/data-service
COPY FridgeFusion.pro /build/

# Create build directories
RUN mkdir -p /build/lib /build/services/data-service/build

# Create dummy config.json
RUN echo '{"database":{"host":"localhost","port":5432,"name":"fridgefusion","username":"postgres","password":""},"api":{"server":{"host":"0.0.0.0","port":8082}},"logging":{"directory":"/app/logs"}}' > /build/config.json

# Build libraries using qmake
WORKDIR /build/libs/core
RUN qmake6 core.pro && make -j$(nproc)

WORKDIR /build/libs/utils
RUN qmake6 utils.pro && make -j$(nproc)

WORKDIR /build/libs/infrastructure
RUN qmake6 infrastructure.pro && make -j$(nproc)

WORKDIR /build/libs/application
RUN qmake6 application.pro && make -j$(nproc)

# Build data-service
WORKDIR /build/services/data-service/build
RUN cmake -DCMAKE_BUILD_TYPE=Release .. && make -j$(nproc)

# Stage 2: Runtime
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libqt6core6 \
    libqt6sql6 \
    libqt6sql6-psql \
    libpq5 \
    libssl3 \
    zlib1g \
    wget \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /build/services/data-service/build/data_service /app/

RUN mkdir -p /app/logs /app/config
RUN chmod +x /app/data_service

EXPOSE 8082

HEALTHCHECK --interval=30s --timeout=3s --start-period=40s --retries=3 \
    CMD wget --spider -q http://localhost:8082/health || exit 1

CMD ["/app/data_service"]
