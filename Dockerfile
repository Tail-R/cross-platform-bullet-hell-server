##### Build Stage ####################################################
FROM debian:bookworm AS builder

# Install build dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git \
    libluajit-5.1-dev \
    ca-certificates \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

# Build
RUN mkdir -p build && cd build \
    && cmake .. \
    && cmake --build .

##### Final Stage ####################################################
FROM debian:bookworm-slim

# Install runtime dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    libluajit-5.1-2 \
    ca-certificates \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/build/bullet_hell_server ./bullet_hell_server

CMD ["./bullet_hell_server"]
