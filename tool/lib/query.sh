# shellcheck shell=bash
# query.sh - read-only commands: list, info, status, versions, targets, examples, where, and the
# hidden __complete used by the shell completion scripts in ../completion.

# list: every SDK with its one-line description.
cmd_list() {
  local sdk
  [[ $# -eq 0 ]] || die_usage list "takes no arguments"
  for sdk in $(sdk_names); do
    printf '%-10s %s\n' "$sdk" "$(plugin_meta "$sdk" desc)"
  done
}

# versions [sdk|all]: pinned/installed versions.
cmd_versions() {
  [[ $# -le 1 ]] || die_usage versions "too many arguments"
  [[ "${1:-all}" == all ]] || require_sdk "$1"
  each_sdk_call sdk_versions "${1:-all}"
}

# targets <sdk>: boards/targets the SDK can build.
cmd_targets() {
  [[ $# -eq 1 ]] || die_usage targets "need one <sdk>"
  require_sdk "$1"
  plugin_call "$1" sdk_targets
}

# examples <sdk>: examples the SDK's build accepts.
cmd_examples() {
  [[ $# -eq 1 ]] || die_usage examples "need one <sdk>"
  require_sdk "$1"
  plugin_call "$1" sdk_examples
}

# where: the key paths of this tool (read-only; creates nothing).
cmd_where() {
  [[ $# -eq 0 ]] || die_usage where "takes no arguments"
  {
    printf 'what\tpath\n'
    printf 'tool\t%s\n' "$MCU_ROOT"
    printf 'output\t%s\n' "$MCU_OUT_BASE"
    printf 'state\t%s\n' "$MCU_OUT_BASE/_state"
    printf 'selfcheck\t%s\n' "$MCU_OUT_BASE/_selfcheck"
    printf 'trash\t%s\n' "${MCU_TRASH:-${UPD_TRASH:-per volume: <mount>/.Trash-$(id -u) or ~/.local/share/Trash}}"
    printf 'completion\t%s\n' "$MCU_ROOT/completion"
  } | print_table
}

# ---------------------------------------------------------------- info

# The plugin commands an SDK supports, as mcu command names.
info_commands() {
  local sdk="$1" fn names=""
  for fn in $(plugin_meta "$sdk" functions); do
    case "$fn" in
      sdk_env | sdk_prune_families | sdk_rollback | sdk_build_dir | sdk_artifacts | sdk_build_outputs | sdk_info) ;;
      sdk_cache_dirs | sdk_clean_cache) names+="clean--cache " ;;
      *) names+="${fn#sdk_} " ;;
    esac
  done
  printf '%s\n' "$names" | tr ' ' '\n' | sed 's/^clean--cache$/clean --cache/' | sort -u | grep . | tr '\n' ' '
  echo
}

# One "label  value" line of `mcu info`; skipped when the value is empty.
info_row() {
  local label="$1" value="$2"
  [[ -n "$value" ]] && printf '  %-13s %s\n' "$label" "$value"
  return 0
}

# Show a file path when it exists.
existing_path() {
  [[ -e "$1" ]] && printf '%s\n' "$1"
  return 0
}

# info <sdk>: description, directories, targets, examples, flash tools, docs.
cmd_info() {
  local sdk="${1:-}" dir api
  [[ $# -eq 1 ]] || die_usage info "need one <sdk>"
  require_sdk "$sdk"
  dir="$MCU_ROOT/$sdk"
  api="v$(plugin_meta "$sdk" api)"
  plugin_is_v2 "$sdk" || api="$api (compat mode)"
  printf '%s - %s\n' "$(paint 1 bold "$sdk")" "$(plugin_meta "$sdk" desc)"
  info_row contract "$api"
  info_row directory "$dir"
  info_row environment "$(existing_path "$dir/env.sh")"
  info_row "build output" "$MCU_OUT_BASE/$sdk"
  info_row readme "$(existing_path "$dir/README.md")"
  info_row versions "$(existing_path "$dir/VERSIONS.md")"
  info_row selfcheck "$(plugin_meta "$sdk" selfcheck)$(plugin_meta "$sdk" no_selfcheck)"
  info_row "default ex." "$(plugin_meta "$sdk" default_example)"
  info_row "flash tools" "$(plugin_meta "$sdk" flash_tools)"
  info_row commands "$(info_commands "$sdk")"
  if plugin_has "$sdk" sdk_info; then
    (plugin_call "$sdk" sdk_info || true) | sed "s/^/  /"
  fi
  if plugin_has "$sdk" sdk_targets; then
    printf '\ntargets:\n'
    (plugin_call "$sdk" sdk_targets || true) | sed "s/^/  /"
  fi
  if plugin_has "$sdk" sdk_examples; then
    printf '\nexamples:\n'
    (plugin_call "$sdk" sdk_examples || true) | sed "s/^/  /"
  fi
}

# ---------------------------------------------------------------- status

# "5m ago" / "3h ago" / "2d ago" for an epoch timestamp.
human_age() {
  local then="$1" delta
  delta=$(($(date +%s) - then))
  if ((delta < 3600)); then
    printf '%dm ago\n' "$((delta / 60))"
  elif ((delta < 172800)); then
    printf '%dh ago\n' "$((delta / 3600))"
  else
    printf '%dd ago\n' "$((delta / 86400))"
  fi
}

# Last build of <sdk>: "OK target/example 2h ago" or "-".
status_last_build() {
  local file epoch target example result
  file="$(state_dir "$1")/last-build"
  [[ -f "$file" ]] || {
    echo "-"
    return 0
  }
  IFS=$'\t' read -r epoch target example result _ <"$file"
  printf '%s %s/%s %s\n' "$result" "$target" "$example" "$(human_age "$epoch")"
}

# Update availability from the last update/outdated check: "yes (3)", "no", "?" (never checked).
status_updates() {
  local file epoch newer unknown
  file="$(state_dir "$1")/update-check"
  [[ -f "$file" ]] || {
    echo "? (run: mcu outdated)"
    return 0
  }
  IFS=$'\t' read -r epoch newer unknown <"$file"
  if ((newer > 0)); then
    printf 'yes (%s) %s\n' "$newer" "$(human_age "$epoch")"
  elif ((unknown > 0)); then
    printf '? %s\n' "$(human_age "$epoch")"
  else
    printf 'no %s\n' "$(human_age "$epoch")"
  fi
}

# Disk use "sdk-dir / build-output", cached for 24h in the state dir (refresh with status --refresh).
status_disk() {
  local sdk="$1" refresh="$2" file epoch sdk_bytes out_bytes
  file="$(state_dir "$sdk")/disk"
  if [[ "$refresh" == 1 || ! -f "$file" ]] || (($(date +%s) - $(cut -f1 "$file") > 86400)); then
    sdk_bytes="$(du -sb "$MCU_ROOT/$sdk" 2>/dev/null | cut -f1)"
    out_bytes="$(du -sb "$MCU_OUT_BASE/$sdk" 2>/dev/null | cut -f1)"
    printf '%s\t%s\t%s\n' "$(date +%s)" "${sdk_bytes:-0}" "${out_bytes:-0}" | write_atomic "$file"
  fi
  IFS=$'\t' read -r epoch sdk_bytes out_bytes <"$file"
  printf '%s / %s\n' "$(human_size "$sdk_bytes")" "$(human_size "$out_bytes")"
}

# First line of sdk_versions, shortened for the table.
status_version() {
  local line
  line="$( (plugin_call "$1" sdk_versions 2>/dev/null || true) | head -1)"
  line="$(tr -s ' \t' ' ' <<<"$line")"
  ((${#line} > 44)) && line="${line:0:41}..."
  printf '%s\n' "${line:--}"
}

# Last selfcheck result: "OK 3h ago" or "-".
status_selfcheck() {
  local file epoch result
  file="$(state_dir "$1")/last-selfcheck"
  [[ -f "$file" ]] || {
    echo "-"
    return 0
  }
  IFS=$'\t' read -r epoch result <"$file"
  printf '%s %s\n' "$result" "$(human_age "$epoch")"
}

# status [sdk|all] [--refresh]: one table with versions, last build/selfcheck, updates, disk use.
cmd_status() {
  local refresh=0 sdk
  local -a words=()
  while [[ $# -gt 0 ]]; do
    case "$1" in
      --refresh) refresh=1 ;;
      -*) die_usage status "unknown option '$1'" ;;
      *) words+=("$1") ;;
    esac
    shift
  done
  [[ ${#words[@]} -le 1 ]] || die_usage status "too many arguments"
  [[ ! -f "$(state_dir "${words[0]:-host}")/disk" || "$refresh" == 1 ]] && log_info "measuring disk use (cached for 24h)"
  {
    printf 'SDK\tVERSION\tLAST BUILD\tSELFCHECK\tUPDATES\tDISK (sdk / out)\n'
    for sdk in $(sdk_selection "${words[0]:-all}"); do
      printf '%s\t%s\t%s\t%s\t%s\t%s\n' "$sdk" "$(status_version "$sdk")" "$(status_last_build "$sdk")" \
        "$(status_selfcheck "$sdk")" "$(status_updates "$sdk")" "$(status_disk "$sdk" "$refresh")"
    done
  } | print_table
}

# ---------------------------------------------------------------- completion

# Cached first-column words of a plugin listing (sdk_targets / sdk_examples); refreshed when sdk.sh changes.
complete_cached_words() {
  local sdk="$1" fn="$2" cache
  sdk_exists "$sdk" || return 0
  plugin_has "$sdk" "$fn" || return 0
  cache="$(state_dir "$sdk")/complete-$fn"
  if [[ ! -s "$cache" || "$MCU_ROOT/$sdk/sdk.sh" -nt "$cache" ]]; then
    (plugin_call "$sdk" "$fn" 2>/dev/null || true) \
      | awk '$1 ~ /^[A-Za-z0-9][A-Za-z0-9_.@\/+-]*$/ && $1 != "TARGET" {print $1}' | write_atomic "$cache"
  fi
  cat "$cache"
}

# Positional words of a partial command line (flags and their values left out).
complete_positionals() {
  local skip_next=0 word
  for word in "$@"; do
    if ((skip_next)); then
      skip_next=0
      continue
    fi
    case "$word" in
      -j | --jobs | --lines) skip_next=1 ;;
      -*) ;;
      *) printf '%s\n' "$word" ;;
    esac
  done
}

# Candidates for the next word after <words...> (the words already typed, without "mcu").
complete_next_word() {
  local -a pos
  local cmd sdk
  mapfile -t pos < <(complete_positionals "$@")
  if [[ ${#pos[@]} -eq 0 ]]; then
    command_names
    return 0
  fi
  cmd="${pos[0]}" sdk="${pos[1]:-}"
  case "${#pos[@]}:$cmd" in
    1:help) command_names ;;
    1:clean | 1:distclean | 1:doctor | 1:versions | 1:selfcheck | 1:libcheck | 1:prune | 1:update | 1:check-plugins | 1:check-clean | 1:status)
      sdk_names
      echo all
      ;;
    1:build | 1:rebuild | 1:flash | 1:monitor | 1:logs | 1:info | 1:targets | 1:examples | 1:shell | 1:run | 1:env | 1:new | 1:rollback) sdk_names ;;
    1:devtools) echo install ;;
    1:ide) echo new ;;
    2:ide) sdk_names ;;
    2:build | 2:rebuild | 2:flash | 2:clean | 2:logs) complete_cached_words "$sdk" sdk_targets ;;
    3:build | 3:rebuild | 3:flash | 3:clean | 3:logs) complete_cached_words "$sdk" sdk_examples ;;
  esac
  return 0
}

# __complete commands|sdks|flags|targets <sdk>|examples <sdk>|next <words...>: words for completion scripts.
cmd___complete() {
  local what="${1:-}"
  shift || true
  case "$what" in
    commands) command_names ;;
    sdks) sdk_names ;;
    flags) printf '%s\n' --yes --dry-run --verbose --quiet --jobs --no-color --help ;;
    targets) complete_cached_words "${1:-}" sdk_targets ;;
    examples) complete_cached_words "${1:-}" sdk_examples ;;
    next) complete_next_word "$@" ;;
  esac
  return 0
}
