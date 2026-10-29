# K230 RT-Smart / CanMV (k230_rtos_sdk) environment. Source from bash or zsh:
#   source toolchain/k230/env.sh   (inside this ID)
# Self-contained (see ../PLUGIN.md): shared ../host layer first (buildtools gcc/make/git/tar/patch/
# wget/pkg-config, cmake, ninja, uv), then this SDK's own tools:
#   .venv (CPython 3.10 uv-managed: scons 3.1.2, pycryptodome, gmssl, jinja2, jsonschema, k230-flash)
#   host/ (bison, flex, m4, mtools, dosfstools, rsync built from source), bin/repo, toolchain/,
#   host/coreutils (GNU coreutils 9.7, last on PATH: only fills gaps like `pwd` left by the OS base set).
if [ -n "${BASH_SOURCE:-}" ]; then _k230_src="${BASH_SOURCE[0]}"
elif [ -n "${ZSH_VERSION:-}" ]; then _k230_src="${(%):-%x}"
else _k230_src="$0"; fi
export K230_HOME="$(cd "$(dirname "$_k230_src")" && pwd)"
unset _k230_src
. "$K230_HOME/../host/env.sh"

export K230_SDK="$K230_HOME/sdk"                       # -> $MCU_OUT_BASE/k230/sdk (fetched there, see README.md)
export SDK_TOOLCHAIN_DIR="$(cd "$K230_HOME/toolchain" && pwd -P)"   # SDK Makefile reads this
export K230_MUSL_BIN="$SDK_TOOLCHAIN_DIR/riscv64-linux-musleabi_for_x86_64-pc-linux-gnu/bin"
export K230_GLIBC_BIN="$SDK_TOOLCHAIN_DIR/Xuantie-900-gcc-linux-5.10.4-glibc-x86_64-V2.6.0/bin"
export K230_DOCKER_IMAGE="${K230_DOCKER_IMAGE:-k230-rtos-build:22.04}"
# scons 3.1.2 (pinned by the SDK) does not run on Python >= 3.12, hence a k230-local 3.10
export SCONS_LIB_DIR="$K230_HOME/.venv/lib/python3.10/site-packages/scons"
# host/ bison and flex were built under an older prefix (paths compiled in); point them at the current tree
export BISON_PKGDATADIR="$K230_HOME/host/share/bison" M4="$K230_HOME/host/bin/m4"
case ":$PATH:" in *":$K230_HOME/bin:"*) ;; *)
  # trailing "$K230_HOME/host/empty": the SDK's toolchain_riscv64_musl.mk does
  # `export PATH:="$(CROSS_COMPILE_DIR):$(PATH)"` (literal quotes) which corrupts the last entry
  export PATH="$K230_HOME/.venv/bin:$K230_HOME/bin:$K230_HOME/host/bin:$K230_HOME/host/sbin:$K230_MUSL_BIN:$PATH:$K230_HOME/host/coreutils/bin:$K230_HOME/host/empty" ;; esac
alias k230-docker="$K230_HOME/k230-docker.sh"
