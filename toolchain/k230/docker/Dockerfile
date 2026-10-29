# K230 RTOS SDK build image.
# Derived from kendryte/k230_rtos_sdk tools/docker/Dockerfile (same apt/pip set),
# but the toolchains are NOT baked in: they live on the host in
# mcu/k230/toolchain and are bind-mounted (SDK_TOOLCHAIN_DIR=/opt/k230_toolchains).
# The container runs as the host user (see ../k230-docker.sh), so no root-owned files.
FROM ubuntu@sha256:0a9887931b0e38af2e3487ed39e29436b0535c4f3f52e57745baaf79d1898956

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    make autoconf automake bison flex gcc g++ gawk libncurses5-dev pkg-config \
    libconfuse-dev libssl-dev python3 python3-pip python-is-python3 cmake \
    libyaml-dev scons mtools bzip2 curl wget git openssh-client rsync dosfstools \
    ca-certificates file cpio bc xz-utils unzip patch && \
    apt-get clean && update-ca-certificates && rm -rf /var/lib/apt/lists/*

RUN pip3 install --no-cache-dir pycryptodome==3.23.0 gmssl==3.2.2 scons==3.1.2 \
    jinja2==3.1.6 markupsafe==2.0.1 jsonschema==4.23.0 pyserial==3.5

ENV SDK_TOOLCHAIN_DIR=/opt/k230_toolchains
WORKDIR /workspace
