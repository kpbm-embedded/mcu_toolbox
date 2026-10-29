# shellcheck shell=bash
# esp-idf build adapter: out-of-tree project, sdkconfig kept in the build dir. Needs `source esp-idf/env.sh` (done by check.sh).
tpl_build() { # srcdir target outdir name
  local src="$1" t="$2" out="$3" b="$3/build"
  idf.py -C "$src" -B "$b" -DSDKCONFIG="$b/sdkconfig" -DIDF_TARGET="$t" build || return 1
  local a; a="$(find "$b" -maxdepth 1 -name '*.bin' -not -name 'bootloader*' -not -name 'partition*' -printf '%s %p\n' | sort -rn | head -1 | cut -d' ' -f2-)"
  [[ -n $a ]] && echo "$a" > "$out/.artifact"
  # host unit tests (Unity) when the project has them
  if [[ -f $src/tests/host/Makefile ]]; then make -C "$src/tests/host" test OUT="$out/host-tests" || return 1; fi
}
