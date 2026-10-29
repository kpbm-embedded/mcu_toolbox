# shellcheck shell=bash
# py32 build adapter: self-contained Makefile in each project; it reads the Puya LL/CMSIS libraries from the template
# checkout read-only (PY32_TEMPLATE, set by py32/env.sh). Output in <outdir>/build/app.{elf,bin,hex}.
tpl_build() { # srcdir target outdir name
  local src="$1" target="$2" out="$3"
  make -C "$src" -j"$MCU_JOBS" TARGET="$target" BUILD="$out/build" PY32="$PY32_TEMPLATE" || return 1
  echo "$out/build/app.bin" > "$out/.artifact"
}
