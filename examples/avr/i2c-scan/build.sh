#!/usr/bin/env bash
# build hook for templates/tools/check.sh: build.sh <target> <outdir>  (env.sh of mcu/avr already sourced by check.sh)
# Arduino sketch dirs are staged under a folder named like the .ino (arduino-cli rule); Makefile dirs use TARGET=<target>.
set -euo pipefail
here="$(cd "$(dirname "$0")" && pwd)"; t="${1:?target}"; out="${2:?outdir}"
case "$t" in
  nano) fqbn=arduino:avr:nano:cpu=atmega328;; nano-old) fqbn=arduino:avr:nano:cpu=atmega328old;;
  mega) fqbn=arduino:avr:mega:cpu=atmega2560;; attiny85) fqbn=ATTinyCore:avr:attinyx5:chip=85,clock=8internal;;
  ch552) fqbn=CH55xDuino:mcs51:ch552;; *) echo "unknown target $t" >&2; exit 2;; esac
if [ -f "$here/Makefile" ]; then
  make -C "$here" TARGET="$t" OUT="$out"
else
  ino="$(ls "$here"/*.ino | head -1)"; n="$(basename "$ino" .ino)"
  stage="$out/stage/$n"; mkdir -p "$stage"; cp "$here"/*.ino "$stage"/
  arduino-cli compile -j "${MCU_JOBS:-16}" -b "$fqbn" --output-dir "$out" "$stage"
fi
