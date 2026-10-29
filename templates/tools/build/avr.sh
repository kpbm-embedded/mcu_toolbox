# shellcheck shell=bash
# avr build adapter (bouffalo uses it too): delegates to the per-project build.sh (which stages a copy; SDK trees untouched). Needs `source <sdk>/env.sh` (done by check.sh).
tpl_build() { # srcdir target outdir name
  local src="$1" t="$2" out="$3"
  [[ -x $src/build.sh ]] || { echo "no $src/build.sh"; return 1; }
  bash "$src/build.sh" "$t" "$out" || return 1
  if [[ ! -s $out/.artifact ]]; then
    local a; a="$(first_file "$out"/*.hex "$out"/*.elf "$out"/*.bin)"; [[ -n $a ]] && echo "$a" > "$out/.artifact"
  fi
  return 0
}
