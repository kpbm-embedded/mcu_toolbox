#!/usr/bin/env bash
# Exit 3 (skip) when the Zig toolchain ID is missing; check.sh then reports SKIP instead of FAIL.
here="$(cd -P "$(dirname "$0")" && pwd -P)"
zb="${BOX_MICROZIG:-$here/../../../../32_24_microzig}/microzig-box"
[ -x "$zb" ] || { echo "SKIP: needs 32_24_microzig (microzig-box) next to this ID or BOX_MICROZIG"; exit 3; }
