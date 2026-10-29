# shellcheck shell=bash
# core.sh - basics shared by every mcu module: settings, logging, prompts, small formatters.
#
# Sourced once by ../mcu (and by host/update-lib.sh for old callers). Defines functions only;
# nothing runs on source. Logging goes to stderr so stdout stays clean for data (tables, paths).

mcu_box_out_of() { # DIR -> DIR's path under the Box `out` location, or nothing (standalone / outside Box)
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

# Fill in every MCU_* setting that the environment did not preset, and export them for plugins.
core_init_settings() {
  if [ -z "${MCU_OUT_BASE:-}" ]; then   # this ID's path under the Box `out` location, else <ID>/build
    local id_dir o; id_dir="$(cd "$MCU_ROOT/.." && pwd -P)"; o="$(mcu_box_out_of "$id_dir")"
    MCU_OUT_BASE="${o:-$id_dir}/build"
  fi
  : "${MCU_JOBS:=16}"
  : "${MCU_YES:=0}"
  : "${MCU_DRY_RUN:=0}"
  : "${MCU_VERBOSE:=0}"
  : "${MCU_QUIET:=0}"
  : "${MCU_COLOR:=auto}"
  export MCU_ROOT MCU_OUT_BASE MCU_JOBS MCU_YES MCU_DRY_RUN MCU_VERBOSE MCU_QUIET MCU_COLOR
}

# Turn on bash strict mode for the dispatcher (plugins inherit it, as before).
core_strict_mode() {
  set -euo pipefail
}

# True when colors should be used on file descriptor $1 (1 = stdout, 2 = stderr).
color_enabled() {
  local fd="$1"
  case "${MCU_COLOR:-auto}" in
    always) return 0 ;;
    never) return 1 ;;
  esac
  [[ -z "${NO_COLOR:-}" && "${TERM:-dumb}" != dumb && -t "$fd" ]]
}

# Wrap text in an ANSI color when fd $1 is a color terminal: paint <fd> <red|green|yellow|blue|dim|bold> <text>.
paint() {
  local fd="$1" color="$2" text="$3" code
  if ! color_enabled "$fd"; then
    printf '%s' "$text"
    return 0
  fi
  case "$color" in
    red) code=31 ;;
    green) code=32 ;;
    yellow) code=33 ;;
    blue) code=34 ;;
    dim) code=2 ;;
    bold) code=1 ;;
    *) code=0 ;;
  esac
  printf '\033[%sm%s\033[0m' "$code" "$text"
}

# Print one log line to stderr with a colored level tag.
log_line() {
  local color="$1" tag="$2"
  shift 2
  printf '%s %s\n' "$(paint 2 "$color" "$tag")" "$*" >&2
}

# Progress/info message; hidden with --quiet.
log_info() {
  [[ "${MCU_QUIET:-0}" == 1 ]] && return 0
  log_line blue "mcu:" "$*"
}

# Detail only shown with --verbose.
log_debug() {
  [[ "${MCU_VERBOSE:-0}" == 1 ]] || return 0
  log_line dim "mcu: [debug]" "$*"
}

# Something worth noticing that does not stop the command.
log_warn() {
  log_line yellow "mcu: warning:" "$*"
}

# A failure message (the caller decides whether to stop).
log_error() {
  log_line red "mcu: error:" "$*"
}

# Stop with an error message (exit 1).
die() {
  log_error "$*"
  exit 1
}

# Stop with a usage error (exit 2) and point at the command help.
die_usage() {
  local cmd="$1"
  shift
  log_error "$*"
  printf 'see: mcu help %s\n' "$cmd" >&2
  exit 2
}

# True when -n/--dry-run is active.
is_dry_run() {
  [[ "${MCU_DRY_RUN:-0}" == 1 ]]
}

# True when -y/--yes (MCU_YES=1) is active and not overridden by --dry-run.
is_yes() {
  [[ "${MCU_YES:-0}" == 1 ]] && ! is_dry_run
}

# Ask a yes/no question; --yes answers yes, a non-interactive stdin without --yes answers no.
confirm() {
  local question="$1" answer
  is_yes && return 0
  if [[ ! -t 0 ]]; then
    log_error "$question -> not confirmed (no terminal); pass --yes to proceed"
    return 1
  fi
  read -r -p "$question [y/N] " answer
  [[ "$answer" == y || "$answer" == Y || "$answer" == yes ]]
}

# Current time in milliseconds (bash 5 EPOCHREALTIME; its decimal mark follows the locale, so keep digits only).
now_ms() {
  local micros="${EPOCHREALTIME//[!0-9]/}"
  printf '%s\n' "$((10#$micros / 1000))"
}

# Format milliseconds as 0.4s / 12.3s / 4m05s.
human_duration() {
  local ms="$1" s
  s=$((ms / 1000))
  if ((s < 60)); then
    printf '%d.%ds\n' "$s" "$(((ms % 1000) / 100))"
  else
    printf '%dm%02ds\n' "$((s / 60))" "$((s % 60))"
  fi
}

# Format a byte count as 812 B / 12.4 KiB / 3.0 MiB / 1.2 GiB.
human_size() {
  local bytes="$1" unit=0 scaled
  local -a units=(B KiB MiB GiB TiB)
  scaled=$((bytes * 10))
  while ((scaled >= 10240 && unit < 4)); do
    scaled=$((scaled / 1024))
    unit=$((unit + 1))
  done
  if ((unit == 0)); then
    printf '%d B\n' "$bytes"
  else
    printf '%d.%d %s\n' "$((scaled / 10))" "$((scaled % 10))" "${units[$unit]}"
  fi
}

# Print TSV from stdin as an aligned table; the first row is the header.
print_table() {
  awk -F'\t' '
    { rows[NR] = $0; if (NF > cols) cols = NF
      for (i = 1; i <= NF; i++) if (length($i) > w[i]) w[i] = length($i) }
    END {
      for (r = 1; r <= NR; r++) {
        n = split(rows[r], f, "\t"); line = ""
        for (i = 1; i <= n; i++) {
          width = w[i] + 2
          line = line (i < n ? sprintf("%-" width "s", f[i]) : f[i])
        }
        print line
      }
    }'
}

# Short state directory for one SDK (records of builds, update checks, caches).
state_dir() {
  local sdk="$1"
  printf '%s/_state/%s\n' "$MCU_OUT_BASE" "$sdk"
}

# Write a file atomically: content from stdin goes to <file>.tmp.<pid>, then mv into place.
write_atomic() {
  local file="$1"
  mkdir -p "$(dirname "$file")"
  cat >"$file.tmp.$$"
  mv -f "$file.tmp.$$" "$file"
}
