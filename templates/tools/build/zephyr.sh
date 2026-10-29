# shellcheck shell=bash
# check.sh build adapter for zephyr. Sourced after zephyr/env.sh. Maps target -> west board from the table in zephyr/sdk.sh.
_z_board() { awk -F'|' -v t="$1" '/^_ZTARGETS=/{f=1; sub(/^_ZTARGETS=./,"")} f&&$1==t{print $2; exit}' "$SDK_DIR/sdk.sh"; }
tpl_build() { # srcdir target outdir name
  local src="$1" t="$2" out="$3" board f; board="$(_z_board "$t")"; board="${board:-$t}"
  CMAKE_BUILD_PARALLEL_LEVEL="${MCU_JOBS:-16}" west build -p auto -b "$board" -d "$out/b" "$src" || return $?
  for f in uf2 bin elf; do [[ -f $out/b/zephyr/zephyr.$f ]] && { echo "$out/b/zephyr/zephyr.$f" > "$out/.artifact"; break; }; done
  return 0
}
