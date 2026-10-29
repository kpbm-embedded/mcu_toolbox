#!/usr/bin/env bash
# build.sh <target> <outdir>: out-of-tree build used by mcu/templates/tools/check.sh (needs bouffalo/env.sh sourced).
# Copies this directory to <outdir>/work (the SDK tree is never touched) and runs the SDK Makefile there.
set -euo pipefail
here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"; t="${1:?target bl616|bl702|bl808}"; out="${2:?outdir}"
base="${BL_SDK_BASE:?source mcu/bouffalo/env.sh first}"
extra=()
case "$t" in
  bl616) chip=bl616; board=bl616dk;;
  bl702) chip=bl702; board=bl702dk;;
  bl808) chip=bl808; board=bl808dk; base="$(dirname "$base")/bouffalo_sdk_legacy"; extra=(CPU_ID=m0);;  # legacy SDK, M0 core
  *) echo "unknown target $t" >&2; exit 2;;
esac
work="$out/work"; mkdir -p "$work"
tar -C "$here" --exclude=./build --exclude=./build.sh --exclude='./*.toml' --exclude=./README.md -cf - . | tar -C "$work" -xf -
# legacy SDK 'make flash' needs flash_prog_cfg.ini next to the Makefile; take it from the SDK example (read only)
[ "$t" = bl808 ] && [ ! -f "$work/flash_prog_cfg.ini" ] && cp "$base/examples/helloworld/flash_prog_cfg.ini" "$work/"
BL_SDK_BASE="$base" make -C "$work" CHIP=$chip BOARD=$board ${extra[@]+"${extra[@]}"} -j"${MCU_JOBS:-16}"
bin="$(ls "$work"/build/build_out/*.bin 2>/dev/null | grep -vE 'boot2|efuse|partition|mfg' | head -1)"
echo "$bin" > "$out/.artifact"
