#!/usr/bin/env bash
# check.sh - build every example and template of one or more SDKs, for every target they list, and print a table.
#
# Examples (mcu/examples/<sdk>/*) are built for each target in their example.toml; templates
# (mcu/templates/<sdk>/*) are instantiated (instantiate.py) and built for each target in their template.toml.
# Each build runs in a subshell with the SDK's own env.sh. Run `check.sh --help` for usage.
#
# Build adapter contract (one per SDK): tools/build/<sdk>.sh, sourced AFTER <sdk>/env.sh, defines
#     tpl_build <srcdir> <target> <outdir> <name>     # build srcdir for target, products under outdir, nonzero on failure
# and may write the main artifact path into <outdir>/.artifact. Fallback if no adapter: <srcdir>/build.sh <target> <outdir>.
# After a successful adapter build, <srcdir>/test.sh <target> <outdir> runs if the example/template has one (exit != 0 = FAIL).
# Env available to adapters: MCU_ROOT SDK_DIR SDK MCU_OUT MCU_JOBS(=16) TOOLS; function first_file (below).
set -uo pipefail
TOOLS="$(cd "$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")" && pwd)"
MCU_ROOT="$(dirname "$(dirname "$TOOLS")")/tool"   # the mcu tool dir (SDK plugins, host/, examples -> ../examples)
box_out_of() { # DIR -> DIR's path under the Box `out` location, or nothing (standalone / outside Box)
  # BOX: the environment, else the folder holding .boxroot above DIR (older trees: the one holding box_env.sh).
  # Base: $BOX_OUT_<NN_NN> (the ID's own [paths] out), else $BOX_OUT, else $BOX_V3 (old name); missing values come
  # from the generated box_env.d/locations.sh. BOX_INTEGRATION=0 forces standalone.
  local dir="$1" box="${BOX:-}" kit=00_09_meta/00_system/00_01_box_env rel id base idv idval="" sub
  [ "${BOX_INTEGRATION:-1}" != 0 ] || return 0
  while [ -z "$box" ] && [ "$dir" != / ] && [ -n "$dir" ]; do
    dir="$(dirname "$dir")"
    if [ -e "$dir/.boxroot" ] || [ -r "$dir/$kit/box_env.sh" ]; then box="$dir"; fi
  done
  [ -n "$box" ] && [ -d "$box" ] || return 0
  box="$(cd -P "$box" && pwd -P)"
  rel="${1#"$box"/}"
  [ "$rel" != "$1" ] || return 0
  id="${rel#*/}"; id="${id#*/}"; id="${id%%/*}"; idv="BOX_OUT_$(printf %.5s "$id")"
  base="${BOX_OUT:-${BOX_V3:-}}"
  eval "idval=\"\${$idv:-}\""
  if { [ -z "$base" ] || [ -z "$idval" ]; } && [ -r "$box/$kit/box_env.d/locations.sh" ]; then
    # shellcheck disable=SC1090,SC1091  # generated per host
    base="${base:-$(BOX="$box"; . "$box/$kit/box_env.d/locations.sh" >/dev/null 2>&1; printf %s "${BOX_OUT:-${BOX_V3:-}}")}"
    # shellcheck disable=SC1090,SC1091
    [ -n "$idval" ] || idval="$(BOX="$box"; . "$box/$kit/box_env.d/locations.sh" >/dev/null 2>&1; eval "printf %s \"\${$idv:-}\"")"
  elif [ -z "$base" ] && [ -r "$box/$kit/box_env.sh" ]; then        # a tree from before locations.sh
    # shellcheck disable=SC2016  # expanded by the child shell
    base="$(BOX="$box" bash -c '. "$1" >/dev/null 2>&1; printf %s "${BOX_V3:-}"' _ "$box/$kit/box_env.sh")"
  fi
  case "$id" in [0-9][0-9]_[0-9][0-9]_*) ;; *) idval="" ;; esac
  if [ -n "$idval" ]; then                                         # the ID's own out (+ DIR's path inside the ID)
    sub="${rel#*/}"; sub="${sub#*/}"
    case "$sub" in */*) sub="${sub#*/}" ;; *) sub="" ;; esac
    printf '%s\n' "$idval${sub:+/$sub}"; return 0
  fi
  if [ -n "$base" ] && [ -d "$base" ]; then printf '%s\n' "$base/$rel"; fi
}
_id_dir="$(cd "$MCU_ROOT/.." && pwd -P)"; _o="$(box_out_of "$_id_dir")"
OUT_BASE="${MCU_EXAMPLES_OUT:-${_o:-$_id_dir}/build/examples}"   # override for side-by-side checks
unset _id_dir _o
export MCU_ROOT TOOLS MCU_JOBS="${MCU_JOBS:-16}"

usage() {
  cat <<'EOF'
check.sh - build every example and template of the given SDKs for every target; print a result table.

usage: check.sh [--only GLOB] [--kind examples|templates] <sdk> [sdk ...]

options:
  --only GLOB    only examples/templates whose name matches GLOB (templates also match as tpl-<name>)
  --kind KIND    examples or templates only (default: both)
  -h, --help     this help

Output: $MCU_EXAMPLES_OUT/<sdk>/<name>/<target>/ (templates: <sdk>/tpl-<name>/<target>/, source in src/),
each with build.log. Prints name/target/OK|FAIL/artifact size; exit 1 on any FAIL, 2 on a usage error.
Works with PATH=mcu/host/sysbin.

environment:
  MCU_EXAMPLES_OUT  output base (default $BOX_OUT/<this ID path>/build/examples)
  MCU_JOBS=16       parallel jobs per build

examples:
  check.sh stm32
  check.sh --kind templates --only 'minimal*' pico wch
EOF
}

# First regular file among the arguments in sorted order (what `ls a b c | head -1` gave), skipping paths that
# contain the --exclude text. Adapters use it to record <outdir>/.artifact.
first_file() {
  local exclude="" file
  if [[ "${1:-}" == --exclude ]]; then exclude="$2"; shift 2; fi
  while IFS= read -r -d '' file; do
    [[ -f "$file" ]] || continue
    [[ -n "$exclude" && "$file" == *"$exclude"* ]] && continue
    printf '%s\n' "$file"
    return 0
  done < <(printf '%s\0' "$@" | sort -z)
  return 1
}

only='*'; kind=both; sdks=()
while [[ $# -gt 0 ]]; do
  case "$1" in
    --only) only="$2"; shift 2;;
    --kind) kind="$2"; shift 2;;
    -h|--help) usage; exit 0;;
    *) sdks+=("$1"); shift;;
  esac
done
[[ ${#sdks[@]} -gt 0 ]] || { echo "usage: check.sh [--only GLOB] [--kind examples|templates] <sdk>... (check.sh --help)" >&2; exit 2; }

set +u   # env.sh files are not nounset-safe
# shellcheck source=SCRIPTDIR/../../tool/host/env.sh
. "$MCU_ROOT/host/env.sh"   # host python3 (tomllib) and toolchain basics
set -u
PY=(python3 -I)
tget() { "${PY[@]}" "$TOOLS/tomlget.py" "$@"; }

rows=(); fails=0; skips=0; logs=()
size_of() { # outdir
  local o="$1" a=""
  [[ -f $o/.artifact ]] && a="$(head -1 "$o/.artifact")"
  if [[ -z $a || ! -f $a ]]; then
    a="$(find "$o" -maxdepth 3 -type f \( -name '*.bin' -o -name '*.uf2' -o -name '*.hex' -o -name '*.elf' -o -name '*.img' -o -name '*.exe' \) -not -path '*/src/*' -printf '%s %p\n' 2>/dev/null | sort -rn | head -1 | cut -d' ' -f2-)"
  fi
  [[ -n $a && -f $a ]] && echo "$(stat -c %s "$a") $(basename "$a")" || echo "- -"
}

build_job() { # sdk label srcdir target outdir name
  local sdk="$1" label="$2" src="$3" target="$4" out="$5" name="$6" rc=0
  mkdir -p "$out"
  # needs.sh: a project that depends on another ID (Rust/Zig toolchains) exits 3 when it is missing -> SKIP
  if [[ -f $src/needs.sh ]] && ! bash "$src/needs.sh" "$target" >"$out/needs.log" 2>&1; then
    skips=$((skips+1))
    rows+=("$(printf '%-9s %-28s %-22s %-5s %8s %s' "$sdk" "$label" "$target" SKIP - "$(head -1 "$out/needs.log")")")
    printf '%s\n' "${rows[-1]}" >&2; return 0
  fi
  (
    set +u
    export SDK="$sdk" SDK_DIR="$MCU_ROOT/$sdk" MCU_OUT="$OUT_BASE/$sdk"
    # shellcheck source=/dev/null  # the SDK's env.sh, chosen at run time
    if [[ -f $SDK_DIR/env.sh ]]; then . "$SDK_DIR/env.sh"; fi
    set -u
    if [[ -f $TOOLS/build/$sdk.sh ]]; then
      # shellcheck source=/dev/null  # the SDK's build adapter, chosen at run time
      . "$TOOLS/build/$sdk.sh"
      tpl_build "$src" "$target" "$out" "$name" || exit $?
      # optional extra checks of the project (size budget, symbols, sections, UF2 header, ...): test.sh <target> <outdir>
      if [[ -f $src/test.sh ]]; then echo "-- test.sh $target"; bash "$src/test.sh" "$target" "$out"; fi
    elif [[ -f $src/build.sh ]]; then
      bash "$src/build.sh" "$target" "$out"
    else
      echo "no adapter tools/build/$sdk.sh and no $src/build.sh"; exit 3
    fi
  ) >"$out/build.log" 2>&1 || rc=$?
  local st=OK sz="- -"
  if [[ $rc -ne 0 ]]; then st=FAIL; fails=$((fails+1)); logs+=("$out/build.log"); else sz="$(size_of "$out")"; fi
  rows+=("$(printf '%-9s %-28s %-22s %-5s %8s %s' "$sdk" "$label" "$target" "$st" "${sz%% *}" "${sz#* }")")
  printf '%s\n' "${rows[-1]}" >&2
}

for sdk in "${sdks[@]}"; do
  [[ -d $MCU_ROOT/$sdk ]] || { echo "unknown sdk $sdk" >&2; fails=$((fails+1)); continue; }
  if [[ $kind != templates ]]; then
    for d in "$MCU_ROOT/examples/$sdk"/*/; do
      [[ -f $d/example.toml ]] || continue
      n="$(basename "$d")"
      # shellcheck disable=SC2053  # $only is a glob (--only) on purpose
      [[ $n == $only ]] || continue
      mapfile -t targets < <(tget "$d/example.toml" targets)
      for t in "${targets[@]}"; do build_job "$sdk" "$n" "${d%/}" "$t" "$OUT_BASE/$sdk/$n/$t" "$n"; done
    done
  fi
  if [[ $kind != examples ]]; then
    for d in "$MCU_ROOT/templates/$sdk"/*/; do
      [[ -f $d/template.toml ]] || continue
      n="$(basename "$d")"
      # shellcheck disable=SC2053  # $only is a glob (--only) on purpose
      [[ "tpl-$n" == $only || $n == $only ]] || continue
      mapfile -t targets < <(tget "$d/template.toml" targets)
      for t in "${targets[@]}"; do
        out="$OUT_BASE/$sdk/tpl-$n/$t"; mkdir -p "$out"
        if [[ -e $out/src ]]; then
          mkdir -p "$HOME/.local/share/Trash/files"; mv "$out/src" "$HOME/.local/share/Trash/files/tplsrc-$sdk-$n-$t-$$-$RANDOM"
        fi
        if ! "${PY[@]}" "$TOOLS/instantiate.py" "$sdk" "$n" "$out/src" --name "chk_${n//-/_}" --target "$t" >"$out/instantiate.log" 2>&1; then
          fails=$((fails+1)); logs+=("$out/instantiate.log")
          rows+=("$(printf '%-9s %-28s %-22s %-5s %8s %s' "$sdk" "tpl:$n" "$t" FAIL - instantiate)"); printf '%s\n' "${rows[-1]}" >&2
          continue
        fi
        build_job "$sdk" "tpl:$n" "$out/src" "$t" "$out" "chk_${n//-/_}"
      done
    done
  fi
done

echo; printf '%-9s %-28s %-22s %-5s %8s %s\n' SDK NAME TARGET RESULT BYTES ARTIFACT
printf '%s\n' "${rows[@]}"
echo "total ${#rows[@]}, failed $fails, skipped $skips"
for l in "${logs[@]}"; do echo "--- $l"; tail -15 "$l"; done
[[ $fails -eq 0 ]]
