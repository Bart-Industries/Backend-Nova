FROM ubuntu:24.04 AS build

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    bison \
    ca-certificates \
    cmake \
    curl \
    flex \
    g++ \
    git \
    linux-libc-dev \
    make \
    ninja-build \
    perl \
    pkg-config \
    python3 \
    unzip \
    zip \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /opt

RUN git clone https://github.com/microsoft/vcpkg.git /opt/vcpkg \
    && /opt/vcpkg/bootstrap-vcpkg.sh -disableMetrics \
    && /opt/vcpkg/vcpkg install "drogon[postgres]:x64-linux"

WORKDIR /app

COPY CMakeLists.txt ./
COPY src ./src

RUN cmake -S . -B build \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake \
    && cmake --build build --config Release

FROM ubuntu:24.04 AS runtime

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    libpq5 \
    libssl3 \
    zlib1g \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=build /opt/vcpkg/installed/x64-linux /opt/vcpkg/installed/x64-linux
COPY --from=build /app/build/nova /app/nova

ENV LD_LIBRARY_PATH=/opt/vcpkg/installed/x64-linux/lib
ENV APP_ENV=production

EXPOSE 8080

CMD ["./nova"]
