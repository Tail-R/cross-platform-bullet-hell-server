FROM archlinux:latest

RUN pacman -Syu --noconfirm \
    base-devel \
    cmake \
    git \
    luajit \
    && pacman -Scc --noconfirm

WORKDIR /app

COPY . .

RUN mkdir -p build && cd build \
    && cmake .. \
    && cmake --build .

CMD ["./build/bullet_hell_server"]
