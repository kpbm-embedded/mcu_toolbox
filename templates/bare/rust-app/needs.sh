#!/usr/bin/env bash
# Exit 3 (skip) when 32_17_embedded_rust is missing (the toolchain is reached there, not copied).
x="${BOX_EMBEDDED_RUST:-{{MCU_ROOT}}/../../32_17_embedded_rust}/rust-box"
[ -x "$x" ] || { echo "SKIP: needs 32_17_embedded_rust (rust-box); set BOX_EMBEDDED_RUST"; exit 3; }
