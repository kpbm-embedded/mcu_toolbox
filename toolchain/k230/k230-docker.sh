#!/usr/bin/env bash
# Run a command in the K230 RTOS SDK build container as the host user.
# Usage: k230-docker.sh [cmd...]   (default: interactive bash in the SDK dir)
#   e.g. k230-docker.sh make k230d_rtos_bpi_zero_defconfig
#        k230-docker.sh make -j16 log
set -euo pipefail
K230_HOME="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SDK="$(readlink -f "$K230_HOME/sdk")"
TC="$(readlink -f "$K230_HOME/toolchain")"
IMAGE="${K230_DOCKER_IMAGE:-k230-rtos-build:22.04}"
[ $# -eq 0 ] && set -- bash
tty_flags=(-i); [ -t 0 ] && tty_flags=(-it)
exec docker run --rm "${tty_flags[@]}" \
  --user "$(id -u):$(id -g)" \
  -e HOME=/tmp -e SDK_TOOLCHAIN_DIR=/opt/k230_toolchains \
  -v "$TC":/opt/k230_toolchains:ro \
  -v "$K230_HOME/bin":/opt/k230bin:ro \
  -e PATH=/opt/k230bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin \
  -v "$SDK":"$SDK" -w "${K230_WORKDIR:-$SDK}" \
  "$IMAGE" "$@"
