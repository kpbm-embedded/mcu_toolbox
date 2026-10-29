# shellcheck shell=bash
# build.sh - build, rebuild, clean, distclean and logs.
#
# The framework owns the build log and the summary: every build is logged to
# $MCU_OUT/<target>/<example>/build.log (mcu-build.log for v1 plugins, which may write their own build.log),
# recorded in $MCU_OUT_BASE/_state/<sdk>/last-build for `mcu status`, and ends with artifacts, duration and OK/FAIL.
# Nothing is ever deleted: clean/distclean move directories to the Trash (trash_move in helpers.sh).

# ---------------------------------------------------------------- paths

# The example to build: the one given, else the plugin's SDK_DEFAULT_EXAMPLE (v1 plugins: "" = plugin decides).
build_pick_example() {
  local sdk="$1" example="${2:-}"
  if [[ -n "$example" ]]; then
    printf '%s\n' "$example"
  else
    plugin_meta "$sdk" default_example
  fi
}

# Output directory of one build: the plugin's sdk_build_dir when it has one, else <out>/<sdk>/<target>/<example>.
build_output_dir() {
  local sdk="$1" target="$2" example="${3:-}"
  if plugin_has "$sdk" sdk_build_dir; then
    plugin_call "$sdk" sdk_build_dir "$target" "$example"
  elif [[ -z "$example" ]]; then
    printf '%s/%s/%s\n' "$MCU_OUT_BASE" "$sdk" "$target"
  else
    printf '%s/%s/%s/%s\n' "$MCU_OUT_BASE" "$sdk" "$target" "${example//\//_}"
  fi
}

# File name of the framework's build log (v1 plugins may already write a build.log of their own).
build_log_name() {
  local sdk="$1"
  if plugin_is_v2 "$sdk"; then echo build.log; else echo mcu-build.log; fi
}

# Top-level entries of $MCU_OUT_BASE/<sdk> that the SDK dir links into (sources kept on the build disk,
# e.g. k230/sdk and k230/toolchain): they are part of the SDK, never build output. "." means the whole dir.
build_protected_entries() {
  local sdk="$1" out="$MCU_OUT_BASE/$1" link target
  while IFS= read -r link; do
    target="$(readlink -f "$link")"
    case "$target" in
      "$out") echo . ;;
      "$out"/*)
        target="${target#"$out"/}"
        echo "${target%%/*}"
        ;;
    esac
  done < <(find "$MCU_ROOT/$sdk/" -maxdepth 2 -type l 2>/dev/null)
  return 0
}

# Every existing path holding build outputs of <sdk>: the per-target dirs in $MCU_OUT (names listed by
# sdk_targets, minus protected entries) plus sdk_build_outputs. Anything else in $MCU_OUT (download caches,
# reference mirrors, logs of the SDK install) is left alone.
build_output_paths() {
  local sdk="$1" out="$MCU_OUT_BASE/$1" protected targets entry name path
  protected="$(build_protected_entries "$sdk" | sort -u)"
  targets="$(complete_cached_words "$sdk" sdk_targets)"
  if [[ -d "$out" ]] && ! grep -qx "[.]" <<<"$protected"; then
    for entry in "$out"/*; do
      name="$(basename "$entry")"
      grep -qxF -- "$name" <<<"$targets" || continue
      grep -qxF -- "$name" <<<"$protected" && continue
      printf '%s\n' "$entry"
    done
  fi
  if plugin_has "$sdk" sdk_build_outputs; then
    while IFS= read -r path; do
      if [[ -n "$path" && -e "$path" ]]; then printf '%s\n' "$path"; fi
    done < <(plugin_call "$sdk" sdk_build_outputs)
  fi
  return 0
}

# ---------------------------------------------------------------- build

# Run the plugin build with output to the console and the log (or the log only with --quiet); sets BUILD_RC.
build_exec() {
  local sdk="$1" log="$2"
  shift 2
  # errexit stays on inside the plugin (as when it is called directly); only this caller tolerates failure
  set +e
  if [[ "${MCU_QUIET:-0}" == 1 ]]; then
    (
      set -e
      plugin_call "$sdk" sdk_build "$@"
    ) >"$log" 2>&1
  else
    (
      set -e
      plugin_call "$sdk" sdk_build "$@"
    ) 2>&1 | tee "$log"
  fi
  BUILD_RC=$?
  set -e
}

# Append the result of one build to the SDK's state (read by `mcu status`).
build_record() {
  local sdk="$1" target="$2" example="$3" result="$4" duration_ms="$5" log="$6" file
  file="$(state_dir "$sdk")/last-build"
  printf '%s\t%s\t%s\t%s\t%s\t%s\n' "$(date +%s)" "$target" "${example:--}" "$result" "$duration_ms" "$log" | write_atomic "$file"
}

# Artifacts of a finished build as "path<TAB>bytes": the plugin's sdk_artifacts, else firmware files in <dir>.
build_artifacts() {
  local sdk="$1" target="$2" example="$3" dir="$4" path
  if plugin_has "$sdk" sdk_artifacts; then
    while IFS= read -r path; do
      [[ -f "$path" ]] && printf '%s\t%s\n' "$path" "$(stat -c %s -- "$path")"
    done < <(plugin_call "$sdk" sdk_artifacts "$target" "$example")
  else
    list_artifacts "$dir"
  fi
}

# Print the end-of-build summary: result, duration, artifacts with sizes, log path.
build_summary() {
  local sdk="$1" target="$2" example="$3" dir="$4" log="$5" rc="$6" duration_ms="$7" path bytes status
  if ((rc == 0)); then status="$(paint 1 green OK)"; else status="$(paint 1 red FAIL)"; fi
  printf '\n%s  %s %s %s  (%s)\n' "$status" "$sdk" "$target" "${example:-(default example)}" "$(human_duration "$duration_ms")"
  if ((rc == 0)); then
    while IFS=$'\t' read -r path bytes; do
      printf '  %-10s %s\n' "$(human_size "$bytes")" "$path"
    done < <(build_artifacts "$sdk" "$target" "$example" "$dir")
  elif [[ "${MCU_QUIET:-0}" == 1 ]]; then
    tail -20 "$log" | sed 's/^/  | /'
  fi
  printf '  log: %s\n' "$log"
}

# Plugin arguments for sdk_build: v2 gets "<target> <example> [extra...]", v1 gets the user's words unchanged.
build_plugin_args() {
  local sdk="$1" target="$2" example="$3"
  shift 3
  if plugin_is_v2 "$sdk"; then
    printf '%s\0' "$target" "$example" "$@"
  else
    printf '%s\0' "$target"
    [[ -n "$example" ]] && printf '%s\0' "$example"
    [[ $# -gt 0 ]] && printf '%s\0' -- "$@"
  fi
  return 0
}

# Build one target: build_run <sdk> <target> <example|""> [extra args for the build tool...].
build_run() {
  local sdk="$1" target="$2" example
  shift 2
  example="$(build_pick_example "$sdk" "${1:-}")"
  shift || true
  local dir log tmp start rc
  local -a args
  dir="$(build_output_dir "$sdk" "$target" "$example")"
  log="$dir/$(build_log_name "$sdk")"
  if is_dry_run; then
    printf 'would build %s %s %s into %s (log: %s)\n' "$sdk" "$target" "${example:-(default)}" "$dir" "$log"
    return 0
  fi
  mapfile -d '' -t args < <(build_plugin_args "$sdk" "$target" "$example" "$@")
  tmp="$(state_dir "$sdk")/build.$$.log"
  mkdir -p "$(dirname "$tmp")"
  log_info "build $sdk $target ${example:-(default example)} -> $dir"
  start="$(now_ms)"
  build_exec "$sdk" "$tmp" "${args[@]}"
  rc="$BUILD_RC"
  mkdir -p "$dir"
  mv -f "$tmp" "$log"
  build_record "$sdk" "$target" "$example" "$( ((rc == 0)) && echo OK || echo FAIL)" "$(($(now_ms) - start))" "$log"
  build_summary "$sdk" "$target" "$example" "$dir" "$log" "$rc" "$(($(now_ms) - start))"
  if [[ "${MCU_VERBOSE:-0}" == 1 ]]; then
    check_clean_sdk "$sdk" || log_warn "the build changed files inside the $sdk SDK (see above)"
  fi
  return "$rc"
}

# Split "<sdk> <target> [example] [-- extra...]" into BUILD_SDK, BUILD_TARGET, BUILD_EXAMPLE and BUILD_EXTRA.
build_parse_args() {
  local cmd="$1"
  shift
  BUILD_SDK="" BUILD_TARGET="" BUILD_EXAMPLE=""
  BUILD_EXTRA=()
  local -a words=()
  while [[ $# -gt 0 ]]; do
    if [[ "$1" == -- ]]; then
      shift
      BUILD_EXTRA=("$@")
      break
    fi
    words+=("$1")
    shift
  done
  [[ ${#words[@]} -ge 2 ]] || die_usage "$cmd" "need <sdk> <target>"
  [[ ${#words[@]} -le 3 ]] || die_usage "$cmd" "too many arguments before '--': ${words[*]:3}"
  BUILD_SDK="${words[0]}" BUILD_TARGET="${words[1]}" BUILD_EXAMPLE="${words[2]:-}"
  require_sdk "$BUILD_SDK"
}

# build <sdk> <target> [example] [-- extra args]
cmd_build() {
  build_parse_args build "$@"
  build_run "$BUILD_SDK" "$BUILD_TARGET" "$BUILD_EXAMPLE" "${BUILD_EXTRA[@]}"
}

# rebuild <sdk> <target> [example] [-- extra args]: clean that build, then build it.
cmd_rebuild() {
  build_parse_args rebuild "$@"
  clean_one_build "$BUILD_SDK" "$BUILD_TARGET" "$BUILD_EXAMPLE"
  build_run "$BUILD_SDK" "$BUILD_TARGET" "$BUILD_EXAMPLE" "${BUILD_EXTRA[@]}"
}

# ---------------------------------------------------------------- clean / distclean

# Show the paths, ask once (unless --yes), then move each to the Trash. --dry-run only lists them.
trash_paths_confirmed() {
  local what="$1" path
  shift
  if [[ $# -eq 0 ]]; then
    echo "nothing to clean ($what)"
    return 0
  fi
  echo "$what:"
  printf '  %s\n' "$@"
  if is_dry_run; then
    echo "(dry run: nothing moved)"
    return 0
  fi
  confirm "Move these ${#}  paths to the Trash?" || {
    echo "aborted"
    return 1
  }
  for path in "$@"; do
    trash_move "$path"
  done
}

# One build dir: the plugin's sdk_clean when it has one (all v1 plugins), else the framework's default.
clean_one_build() {
  local sdk="$1" target="$2" example
  example="$(build_pick_example "$sdk" "${3:-}")"
  if plugin_has "$sdk" sdk_clean; then
    if is_dry_run; then
      echo "would run: $sdk sdk_clean $target ${example}"
      return 0
    fi
    plugin_call "$sdk" sdk_clean "$target" ${example:+"$example"}
    return
  fi
  local dir
  dir="$(build_output_dir "$sdk" "$target" "$example")"
  if [[ -e "$dir" ]]; then
    trash_move "$dir" "mcu-$sdk-$target-${example//\//_}"
  else
    echo "nothing to clean at $dir"
  fi
}

# Every build output of the selected SDKs (the whole $MCU_OUT/<sdk> plus sdk_build_outputs).
clean_builds() {
  local which="$1" sdk
  local -a paths=()
  for sdk in $(sdk_selection "$which"); do
    mapfile -t -O "${#paths[@]}" paths < <(build_output_paths "$sdk")
  done
  trash_paths_confirmed "build outputs of $which" "${paths[@]}"
}

# SDK caches (download caches, ccache...): sdk_cache_dirs lists them, or the plugin's own sdk_clean_cache.
clean_caches() {
  local which="$1" sdk path
  local -a paths=()
  for sdk in $(sdk_selection "$which"); do
    if plugin_has "$sdk" sdk_clean_cache; then
      if is_dry_run; then
        echo "would run: $sdk sdk_clean_cache"
      elif confirm "Clean the caches of $sdk (sdk_clean_cache)?"; then
        plugin_call "$sdk" sdk_clean_cache
      fi
    elif plugin_has "$sdk" sdk_cache_dirs; then
      while IFS= read -r path; do
        [[ -n "$path" && -e "$path" ]] && paths+=("$path")
      done < <(plugin_call "$sdk" sdk_cache_dirs)
    elif [[ "$which" != all ]]; then
      echo "$sdk: the plugin defines no caches (sdk_cache_dirs)"
    fi
  done
  [[ ${#paths[@]} -eq 0 && "$which" != all ]] && return 0
  trash_paths_confirmed "caches of $which" "${paths[@]}"
}

# clean <sdk> <target> [example] | clean <sdk> | clean all | clean --cache <sdk|all>
cmd_clean() {
  local cache=0
  local -a words=()
  while [[ $# -gt 0 ]]; do
    case "$1" in
      --cache) cache=1 ;;
      -*) die_usage clean "unknown option '$1'" ;;
      *) words+=("$1") ;;
    esac
    shift
  done
  [[ ${#words[@]} -ge 1 ]] || die_usage clean "need <sdk> [target [example]] or 'all'"
  if ((cache)); then
    [[ ${#words[@]} -eq 1 ]] || die_usage clean "--cache takes one <sdk|all>"
    clean_caches "${words[0]}"
  elif [[ ${#words[@]} -eq 1 ]]; then
    clean_builds "${words[0]}"
  else
    [[ ${#words[@]} -le 3 ]] || die_usage clean "too many arguments"
    require_sdk "${words[0]}"
    clean_one_build "${words[@]}"
  fi
}

# distclean <sdk|all>: every build output, selfcheck output and state record of the SDK (not its sources/venvs).
cmd_distclean() {
  local which="${1:-}" sdk path
  local -a paths=()
  [[ $# -eq 1 ]] || die_usage distclean "need one <sdk|all>"
  for sdk in $(sdk_selection "$which"); do
    mapfile -t -O "${#paths[@]}" paths < <(build_output_paths "$sdk")
    for path in "$MCU_OUT_BASE/_selfcheck/$sdk" "$MCU_OUT_BASE/_selfcheck/$sdk.log" "$(state_dir "$sdk")"; do
      [[ -e "$path" ]] && paths+=("$path")
    done
  done
  trash_paths_confirmed "build, selfcheck and state outputs of $which" "${paths[@]}"
}

# ---------------------------------------------------------------- logs

# Newest build log under <dir> (build.log or mcu-build.log), empty when there is none.
logs_newest_in() {
  local dir="$1"
  [[ -d "$dir" ]] || return 0
  find "$dir" -maxdepth 4 -type f \( -name build.log -o -name mcu-build.log \) -printf '%T@\t%p\n' 2>/dev/null \
    | sort -n | tail -1 | cut -f2
}

# The log `mcu logs` shows: the newest build log of the scope, or the selfcheck log when that is newer.
logs_pick() {
  local sdk="$1" target="$2" example="$3" dir candidate selfcheck_log
  if [[ -n "$example" ]]; then
    dir="$(build_output_dir "$sdk" "$target" "$example")"
  elif [[ -n "$target" ]]; then
    dir="$MCU_OUT_BASE/$sdk/$target"
  else
    dir="$MCU_OUT_BASE/$sdk"
  fi
  candidate="$(logs_newest_in "$dir")"
  selfcheck_log="$MCU_OUT_BASE/_selfcheck/$sdk.log"
  if [[ -z "$target" && -f "$selfcheck_log" && (-z "$candidate" || "$selfcheck_log" -nt "$candidate") ]]; then
    candidate="$selfcheck_log"
  fi
  printf '%s\n' "$candidate"
}

# logs <sdk> [target] [example] [--lines N]: path of the last build/selfcheck log and its tail.
cmd_logs() {
  local lines=40 log
  local -a words=()
  while [[ $# -gt 0 ]]; do
    case "$1" in
      --lines) lines="${2:?--lines needs a number}" && shift ;;
      --lines=*) lines="${1#--lines=}" ;;
      -*) die_usage logs "unknown option '$1'" ;;
      *) words+=("$1") ;;
    esac
    shift
  done
  [[ ${#words[@]} -ge 1 && ${#words[@]} -le 3 ]] || die_usage logs "need <sdk> [target] [example]"
  require_sdk "${words[0]}"
  log="$(logs_pick "${words[0]}" "${words[1]:-}" "${words[2]:-}")"
  [[ -n "$log" ]] || die "no build or selfcheck log for ${words[*]}"
  printf 'log: %s (%s)\n' "$log" "$(date -r "$log" '+%F %T')"
  tail -n "$lines" "$log"
}
