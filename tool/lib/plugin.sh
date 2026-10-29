# shellcheck shell=bash
# plugin.sh - finding, loading and validating SDK plugins (<sdk>/sdk.sh, contract in ../PLUGIN.md).
#
# Every plugin function runs in a subshell (plugin_call), so one SDK's environment never leaks into another.
# Plugin facts (description, API version, functions...) are read once per run by plugin_meta and cached.
# Contract v2 plugins set SDK_API=2; plugins without it run in compat mode (v1) until they are migrated.

declare -gA PLUGIN_META=()

# Names of every SDK in the workspace (directories holding sdk.sh), sorted.
sdk_names() {
  local dir
  for dir in "$MCU_ROOT"/*/; do
    [[ -f "$dir/sdk.sh" ]] && basename "$dir"
  done
  return 0
}

# True when <name> is an SDK of this workspace.
sdk_exists() {
  local name="$1"
  [[ -n "$name" && "$name" != */* && -f "$MCU_ROOT/$name/sdk.sh" ]]
}

# Stop unless <name> is an SDK (usage error with the list of SDKs).
require_sdk() {
  local name="${1:-}"
  sdk_exists "$name" && return 0
  die "unknown sdk '${name}' (have: $(sdk_names | tr '\n' ' '))"
}

# Expand "all" (or nothing) to every SDK; otherwise validate the one given.
sdk_selection() {
  local which="${1:-all}"
  if [[ "$which" == all ]]; then
    sdk_names
  else
    require_sdk "$which"
    printf '%s\n' "$which"
  fi
}

# Source the framework helpers and the plugin into the current (sub)shell, with the plugin variables exported.
plugin_enter() {
  local sdk="$1"
  export SDK="$sdk" SDK_DIR="$MCU_ROOT/$sdk" MCU_OUT="$MCU_OUT_BASE/$sdk"
  cd "$SDK_DIR" || return 1
  # shellcheck source=lib/update.sh
  source "$MCU_ROOT/lib/update.sh"
  # shellcheck source=lib/helpers.sh
  source "$MCU_ROOT/lib/helpers.sh"
  # shellcheck source=/dev/null
  source "$SDK_DIR/sdk.sh"
}

# Run plugin function <fn> of <sdk> in a subshell, after sdk_env: plugin_call <sdk> <fn> [args...].
# <fn> can also be a helper (target_names, list_artifacts...) that needs the plugin loaded.
plugin_call() {
  local sdk="$1" fn="$2"
  shift 2
  require_sdk "$sdk"
  log_debug "call $sdk $fn $*"
  (
    plugin_enter "$sdk"
    declare -F "$fn" >/dev/null || die "$sdk: '$fn' is not supported by this plugin"
    if [[ "$fn" != sdk_env ]] && declare -F sdk_env >/dev/null; then
      sdk_env
    fi
    "$fn" "$@"
  )
}

# Read the plugin's facts once (in a throw-away bash) and cache them in PLUGIN_META["<sdk>:<key>"].
# Keys: desc api selfcheck no_selfcheck default_example flash_tools functions defined.
plugin_meta_load() {
  local sdk="$1" fact value loaded="$1:loaded"
  [[ -n "${PLUGIN_META[$loaded]:-}" ]] && return 0
  while IFS=$'\t' read -r fact value; do
    PLUGIN_META["$sdk:$fact"]="$value"
  done < <(plugin_meta_dump "$sdk")
  PLUGIN_META["$loaded"]=1
}

# Print "key<TAB>value" facts of one plugin (runs in a separate bash, no sdk_env).
plugin_meta_dump() {
  local sdk="$1"
  (
    export SDK="$sdk" SDK_DIR="$MCU_ROOT/$sdk" MCU_OUT="$MCU_OUT_BASE/$sdk"
    cd "$SDK_DIR" || exit 1
    bash -c '
      source "$1/lib/update.sh"; source "$1/lib/helpers.sh"
      before="$(declare -F | awk "{print \$3}")"
      source ./sdk.sh >/dev/null 2>&1
      after="$(declare -F | awk "{print \$3}")"
      printf "desc\t%s\n" "${SDK_DESC:-}"
      printf "api\t%s\n" "${SDK_API:-1}"
      printf "selfcheck\t%s\n" "${SDK_SELFCHECK:-}"
      printf "no_selfcheck\t%s\n" "${SDK_NO_SELFCHECK:-}"
      printf "default_example\t%s\n" "${SDK_DEFAULT_EXAMPLE:-}"
      printf "flash_tools\t%s\n" "${SDK_FLASH_TOOLS:-}"
      printf "functions\t%s\n" "$(grep "^sdk_" <<<"$after" | tr "\n" " ")"
      printf "defined\t%s\n" "$(comm -13 <(sort <<<"$before") <(sort <<<"$after") | grep "^sdk_" | tr "\n" " ")"
    ' _ "$MCU_ROOT"
  )
}

# One cached fact of a plugin: plugin_meta <sdk> <key>.
plugin_meta() {
  local sdk="$1" fact="$1:$2"
  plugin_meta_load "$sdk"
  printf '%s\n' "${PLUGIN_META[$fact]:-}"
}

# True when the plugin (or the default API) provides function <fn>.
plugin_has() {
  local sdk="$1" fn="$2"
  [[ " $(plugin_meta "$sdk" functions) " == *" $fn "* ]]
}

# True for contract v2 plugins (SDK_API=2).
plugin_is_v2() {
  [[ "$(plugin_meta "$1" api)" == 2 ]]
}

# ---------------------------------------------------------------- check-plugins

# Plugin source lines without full-line comments: "lineno:text".
plugin_code_lines() {
  local file="$1"
  [[ -f "$file" ]] || return 0
  grep -nvE '^[[:space:]]*#' "$file" || true
}

# Lines calling rm (the workspace rule: move to the Trash instead).
plugin_find_rm() {
  plugin_code_lines "$1" | grep -E '(^[0-9]+:|[[:space:];&|({`])rm[[:space:]]' || true
}

# Lines calling OS tools by absolute path (/usr/bin/..., /bin/...).
plugin_find_os_paths() {
  plugin_code_lines "$1" | grep -E '(^[0-9]+:|[[:space:]=(:]|[[:space:]=(]["'"'"'])/(usr/)?s?bin/' || true
}

# Lines writing to the Trash by hand instead of trash_move (v2 only).
plugin_find_manual_trash() {
  plugin_code_lines "$1" | grep -E 'share/Trash' || true
}

# Print one check-plugins result line and remember failures in CHECK_RC.
check_report() {
  local level="$1" message="$2"
  printf '%-4s %-9s %s\n' "$level" "$CHECK_SDK" "$message"
  [[ "$level" == FAIL ]] && CHECK_RC=1
  return 0
}

# Static checks on the plugin sources: no rm, no OS tools by absolute path, no hand-made Trash moves (v2).
check_plugin_sources() {
  local sdk="$1" file="$MCU_ROOT/$1/sdk.sh" env_file="$MCU_ROOT/$1/env.sh" hits
  hits="$(
    plugin_find_rm "$file"
    plugin_find_rm "$env_file"
  )"
  [[ -z "$hits" ]] || check_report FAIL "calls rm (use trash_move): $(head -1 <<<"$hits")"
  hits="$(
    plugin_find_os_paths "$file"
    plugin_find_os_paths "$env_file"
  )"
  [[ -z "$hits" ]] || check_report FAIL "calls OS tools by path: $(head -1 <<<"$hits")"
  if plugin_is_v2 "$sdk"; then
    hits="$(plugin_find_manual_trash "$file")"
    [[ -z "$hits" ]] || check_report FAIL "writes to the Trash by hand (use trash_move): $(head -1 <<<"$hits")"
  fi
}

# Contract checks on what the plugin defines: variables and functions.
check_plugin_contract() {
  local sdk="$1" fn level=WARN
  [[ -n "$(plugin_meta "$sdk" desc)" ]] || check_report FAIL "SDK_DESC is empty"
  for fn in sdk_env sdk_doctor sdk_versions; do
    plugin_has "$sdk" "$fn" || check_report FAIL "required function $fn is missing"
  done
  if plugin_has "$sdk" sdk_build; then
    for fn in sdk_targets sdk_examples; do
      plugin_has "$sdk" "$fn" || check_report FAIL "$fn is required when sdk_build exists"
    done
  fi
  plugin_is_v2 "$sdk" && level=FAIL
  if [[ -z "$(plugin_meta "$sdk" selfcheck)$(plugin_meta "$sdk" no_selfcheck)" ]] && ! plugin_has "$sdk" sdk_selfcheck; then
    check_report "$level" "no SDK_SELFCHECK, sdk_selfcheck or SDK_NO_SELFCHECK reason"
  fi
  plugin_is_v2 "$sdk" || return 0
  if plugin_has "$sdk" sdk_build && [[ -z "$(plugin_meta "$sdk" default_example)" ]]; then
    check_report FAIL "SDK_DEFAULT_EXAMPLE is required when sdk_build exists"
  fi
  [[ -f "$MCU_ROOT/$sdk/env.sh" ]] || check_report WARN "no env.sh (mcu shell/run/env need one)"
}

# Validate one plugin; prints "OK|WARN|FAIL <sdk> <message>" lines; returns 1 when anything failed.
check_plugin() {
  local sdk="$1"
  CHECK_SDK="$sdk"
  CHECK_RC=0
  if ! bash -n "$MCU_ROOT/$sdk/sdk.sh" 2>/dev/null; then
    check_report FAIL "syntax error in sdk.sh (bash -n)"
    return 1
  fi
  check_plugin_contract "$sdk"
  check_plugin_sources "$sdk"
  if ((CHECK_RC == 0)); then
    if plugin_is_v2 "$sdk"; then check_report OK "contract v2"; else check_report OK "contract v1 (compat mode; migrate to SDK_API=2, see PLUGIN.md)"; fi
  fi
  return "$CHECK_RC"
}

# check-plugins [sdk|all]: validate plugins against the contract.
cmd_check_plugins() {
  local sdk rc=0
  [[ $# -le 1 ]] || die_usage check-plugins "too many arguments"
  for sdk in $(sdk_selection "${1:-all}"); do
    check_plugin "$sdk" || rc=1
  done
  return "$rc"
}
