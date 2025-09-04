##### Build Stage ####################################################
FROM archlinux:latest AS builder

# Build dependencies
RUN pacman -Syu --noconfirm \
    base-devel \
    cmake \
    git \
    luajit \
    && pacman -Scc --noconfirm

WORKDIR /app
COPY . .

# Build
RUN mkdir -p build && cd build \
    && cmake .. \
    && cmake --build .

##### Final Stage ####################################################
FROM archlinux:latest

# Runtime dependencies only
RUN pacman -Syu --noconfirm \
    luajit \
    && pacman -Scc --noconfirm

WORKDIR /app

# Copy built binary from builder
COPY --from=builder /app/build/bullet_hell_server ./bullet_hell_server

CMD ["./bullet_hell_server"]
