#!/usr/bin/env bash
# Exit 3 (skip) when the Rust toolchain ID is missing; check.sh then reports SKIP instead of FAIL.
here="$(cd -P "$(dirname "$0")" && pwd -P)"
rb="${BOX_EMBEDDED_RUST:-$here/../../../../32_17_embedded_rust}/rust-box"
[ -x "$rb" ] || { echo "SKIP: needs 32_17_embedded_rust (rust-box) next to this ID or BOX_EMBEDDED_RUST"; exit 3; }
