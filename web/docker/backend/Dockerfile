# Multi-stage build for FridgeFusion C++ Backend

# Stage 1: Builder
FROM ubuntu:22.04 AS builder

# Set non-interactive mode for apt
ENV DEBIAN_FRONTEND=noninteractive

# Install build dependencies
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

# Set working directory
WORKDIR /build

# Copy source code
COPY libs /build/libs
COPY web_api /build/web_api
COPY third_party /build/third_party
COPY FridgeFusion.pro /build/

# Create build directories
RUN mkdir -p /build/lib /build/web_api/build

# Create dummy config.json for CMake (actual config will be mounted at runtime)
RUN echo '{"database":{"host":"localhost","port":5432,"name":"fridgefusion","username":"postgres","password":""},"api":{"server":{"host":"0.0.0.0","port":8080}},"logging":{"directory":"/app/logs"}}' > /build/config.json

# Build libraries using qmake
# Note: .pro files have DESTDIR set to ../../lib, so .a files go there automatically
WORKDIR /build/libs/core
RUN qmake6 core.pro && make -j$(nproc)

WORKDIR /build/libs/utils
RUN qmake6 utils.pro && make -j$(nproc)

WORKDIR /build/libs/infrastructure
RUN qmake6 infrastructure.pro && make -j$(nproc)

WORKDIR /build/libs/application
RUN qmake6 application.pro && make -j$(nproc)

# Build web_api using CMake
WORKDIR /build/web_api/build

# Fix library linking order for Linux (GNU ld requires dependency order: consumers before providers)
# macOS linker works with any order, but Linux doesn't
# Change order from: utils, application, infrastructure, core
# To:                application, infrastructure, utils, core
RUN sed -i 's|\${LIB_DIR}/libutils.a|__TEMP_UTILS__|' ../CMakeLists.txt && \
    sed -i 's|\${LIB_DIR}/libapplication.a|\${LIB_DIR}/libutils.a|' ../CMakeLists.txt && \
    sed -i 's|\${LIB_DIR}/libinfrastructure.a|\${LIB_DIR}/libapplication.a|' ../CMakeLists.txt && \
    sed -i 's|__TEMP_UTILS__|\${LIB_DIR}/libinfrastructure.a|' ../CMakeLists.txt

RUN cmake -DCMAKE_BUILD_TYPE=Release .. && make -j$(nproc)

# Stage 2: Runtime
FROM ubuntu:22.04

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libqt6core6 \
    libqt6sql6 \
    libqt6sql6-psql \
    libpq5 \
    libssl3 \
    zlib1g \
    wget \
    && rm -rf /var/lib/apt/lists/*

# Create app directory
WORKDIR /app

# Copy binary from builder
COPY --from=builder /build/web_api/build/fridgefusion_api /app/

# Copy swagger UI
COPY --from=builder /build/web_api/build/swagger_ui /app/swagger_ui

# Create logs directory
RUN mkdir -p /app/logs /app/config

# Set executable permissions
RUN chmod +x /app/fridgefusion_api

# Expose port (will be overridden by config)
EXPOSE 8080

# Health check endpoint
HEALTHCHECK --interval=30s --timeout=3s --start-period=40s --retries=3 \
    CMD wget --spider -q http://localhost:8080/api/v2/health || exit 1

# Run the application
CMD ["/app/fridgefusion_api"]
