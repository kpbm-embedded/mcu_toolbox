#!/usr/bin/env bash
# Build the host tools the K230 SDK needs into mcu/k230/host (no OS packages):
#   python 3.10 (uv-managed) + .venv (scons 3.1.2, pycryptodome, gmssl, jinja2, jsonschema, cmake, pyserial, k230-flash)
#   m4, bison, flex (kconfig parser), mtools + dosfstools (vfat partitions in the SD image), rsync (SDK sync_dir)
# Sources: ftp.gnu.org, github.com/westes/flex, github.com/dosfstools, download.samba.org (checksums below).
# Compiled with the shared ../host buildtools gcc/make. Idempotent: skips tools already installed.
set -euo pipefail
K230_HOME="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PREFIX="$K230_HOME/host"
DL="${K230_DL:-${MCU_OUT_BASE:-$(cd "$(dirname "$0")/../../.." && pwd)/build}/k230/dl/src}"
BLD="${K230_HOSTBUILD:-${MCU_OUT_BASE:-$(cd "$(dirname "$0")/../../.." && pwd)/build}/k230/dl/build}"
JOBS="${MCU_JOBS:-16}"
mkdir -p "$PREFIX" "$DL" "$BLD"
. "$K230_HOME/../host/env.sh"   # buildtools gcc/make, no OS compiler
export PATH="$PREFIX/bin:$PATH"
export CFLAGS="-O2 -std=gnu17"

fetch() { # url sha256
  local f="$DL/$(basename "$1")"
  [[ -f "$f" ]] || curl -sfL --connect-timeout 30 --retry 3 -o "$f" "$1"
  echo "$2  $f" | sha256sum -c --quiet -
}
build() { # name url sha256 check-binary [configure args...]
  local name="$1" url="$2" sum="$3" bin="$4"; shift 4
  local pfx="${PFX:-$PREFIX}"
  if [[ -x "$pfx/bin/$bin" || -x "$pfx/sbin/$bin" ]]; then echo "ok $name"; return; fi
  fetch "$url" "$sum"
  local src="$BLD/$name"; [[ -d "$src" ]] && mv "$src" "$HOME/.local/share/Trash/files/k230-hb-$name-$(date +%s)"
  mkdir -p "$src" && tar xf "$DL/$(basename "$url")" -C "$src" --strip-components=1
  ( cd "$src" && ./configure --prefix="$pfx" --disable-nls "$@" >configure.log 2>&1 \
      && make -j"$JOBS" >make.log 2>&1 && make install >install.log 2>&1 ) \
    || { echo "FAILED $name (logs in $src)"; exit 1; }
  echo "built $name"
}

build m4-1.4.19 https://ftp.gnu.org/gnu/m4/m4-1.4.19.tar.xz \
  63aede5c6d33b6d9b13511cd0be2cac046f2e70fd0a07aa9573a04a82783af96 m4
build bison-3.8.2 https://ftp.gnu.org/gnu/bison/bison-3.8.2.tar.xz \
  9bba0214ccf7f1079c5d59210045227bcf619519840ebfa80cd3849cff5a5bf2 bison M4="$PREFIX/bin/m4" \
  ac_cv_libtextstyle=no --without-libtextstyle-prefix   # no OS libtextstyle/ncurses
build flex-2.6.4 https://github.com/westes/flex/releases/download/v2.6.4/flex-2.6.4.tar.gz \
  e87aae032bf07c26f85ac0ed3250998c37621d95f8bd748b31f15b33c45ee995 flex M4="$PREFIX/bin/m4"
build mtools-4.0.48 https://ftp.gnu.org/gnu/mtools/mtools-4.0.48.tar.bz2 \
  03c29aac8735dd7154a989fbc29eaf2b506121ae1c3a35cd0bf2a02e94d271a9 mcopy --without-x
build dosfstools-4.2 https://github.com/dosfstools/dosfstools/releases/download/v4.2/dosfstools-4.2.tar.gz \
  64926eebf90092dca21b14259a5301b7b98e7b1943e8a201c7d726084809b527 mkdosfs --enable-compat-symlinks
build rsync-3.4.1 https://download.samba.org/pub/rsync/src/rsync-3.4.1.tar.gz \
  2924bcb3a1ed8b551fc101f740b9f0fe0a202b115027647cf69850d65fd88c52 rsync \
  --disable-openssl --disable-xxhash --disable-zstd --disable-lz4 --disable-md2man \
  --disable-acl-support --disable-xattr-support --with-included-popt   # glibc only
# GNU coreutils in its own prefix, appended to PATH *after* the OS base utils: only fills gaps
# such as `pwd` (GNU make's $(shell pwd) execs it) that are not in ../host/sysbin.list
PFX="$PREFIX/coreutils" build coreutils-9.7 https://ftp.gnu.org/gnu/coreutils/coreutils-9.7.tar.xz \
  e8bb26ad0293f9b5a1fc43fb42ba970e312c66ce92c1b0b16713d7500db251bf pwd --without-openssl --without-selinux --disable-libcap

# Python: uv-managed CPython 3.10 (same minor as the SDK's Ubuntu 22.04; scons 3.1.2 needs <3.12)
export UV_PYTHON_INSTALL_DIR="$PREFIX/python" UV_CACHE_DIR="$HOME/.cache/uv"
if [[ ! -x "$K230_HOME/.venv/bin/scons" ]]; then
  uv python install --no-bin 3.10.22 >/dev/null
  [[ -d "$K230_HOME/.venv" ]] && mv "$K230_HOME/.venv" "$HOME/.local/share/Trash/files/k230-venv-$(date +%s)"
  uv venv -q --python "$(uv python find 3.10.22)" "$K230_HOME/.venv"
  uv pip install -q --python "$K230_HOME/.venv/bin/python" -r "$K230_HOME/scripts/requirements.txt"
fi
echo "ok python $("$K230_HOME/.venv/bin/python" --version)"

