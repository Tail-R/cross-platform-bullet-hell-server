##### Build Stage ####################################################
FROM alpine:latest AS builder

# Install dependencies
RUN apk add --no-cache \
    build-base \
    cmake \
    git \
    luajit-dev \
    musl-dev \
    libc-dev

WORKDIR /app

COPY . .

# Build
RUN mkdir -p build && cd build \
    && cmake .. \
    && cmake --build .

##### Final Stage ####################################################
FROM scratch

WORKDIR /app

COPY --from=builder /app/build/bullet_hell_server ./bullet_hell_server

CMD ["./bullet_hell_server"]
