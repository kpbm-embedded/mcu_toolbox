#!/usr/bin/env bash
# Exit 3 (skip) when 32_24_microzig is missing (the toolchain is reached there, not copied).
x="${BOX_MICROZIG:-{{MCU_ROOT}}/../../32_24_microzig}/microzig-box"
[ -x "$x" ] || { echo "SKIP: needs 32_24_microzig (microzig-box); set BOX_MICROZIG"; exit 3; }
