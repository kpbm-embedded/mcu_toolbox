# shellcheck shell=bash
# helpers.sh - the helper API for plugins (<sdk>/sdk.sh). Sourced in every plugin subshell, after update.sh.
#
# Plugins call these instead of hand-rolling the same patterns: doctor lines, version lines, tool checks,
# target validation, CMake/make builds, artifact listing and moving things to the Trash.
# Everything is set -u safe and works with PATH limited to host/sysbin (selfcheck strict mode).

# ---------------------------------------------------------------- doctor / versions output

# One doctor line: print_ok_missing <label> <0=ok|1=missing> [detail]; returns the status it was given.
print_ok_missing() {
  local label="$1" status="$2" detail="${3:-}"
  if [[ "$status" == 0 ]]; then
    printf 'ok      %-20s %s\n' "$label" "$detail"
  else
    printf 'MISSING %-20s %s\n' "$label" "$detail"
  fi
  return "$status"
}

# A non-fatal doctor note (optional tool absent, unusual setup).
print_warn() {
  local label="$1" detail="${2:-}"
  printf 'warn    %-20s %s\n' "$label" "$detail"
}

# One version line: print_version <component> <version>.
print_version() {
  local component="$1" version="$2"
  printf '%-20s %s\n' "$component" "$version"
}

# First line of `<tool> [args]` output ("" when the tool fails).
tool_version() {
  { "$@" 2>&1 </dev/null || true; } | head -1
}

# True when <tool> is on PATH.
has_tool() {
  command -v "$1" >/dev/null 2>&1
}

# Fail with a hint when <tool> is not on PATH: require_tool <tool> [hint].
require_tool() {
  local tool="$1" hint="${2:-check: mcu doctor ${SDK:-<sdk>}}"
  has_tool "$tool" && return 0
  echo "$tool not found ($hint)" >&2
  return 1
}

# Doctor line for a command: doctor_tool <label> <tool> [version args, default --version].
doctor_tool() {
  local label="$1" tool="$2"
  shift 2
  [[ $# -gt 0 ]] || set -- --version
  if has_tool "$tool"; then
    print_ok_missing "$label" 0 "$(tool_version "$tool" "$@")"
  else
    print_ok_missing "$label" 1
  fi
}

# Doctor line for a file or directory: doctor_path <label> <path> [detail].
doctor_path() {
  local label="$1" path="$2" detail="${3:-}"
  if [[ -e "$path" ]]; then
    print_ok_missing "$label" 0 "$detail"
  else
    print_ok_missing "$label" 1 "$path"
  fi
}

# Doctor line for a git checkout, showing its tag or commit: doctor_git <label> <dir>.
doctor_git() {
  local label="$1" dir="$2"
  if [[ -d "$dir/.git" || -f "$dir/.git" ]]; then
    print_ok_missing "$label" 0 "$(git_ref "$dir")"
  else
    print_ok_missing "$label" 1 "$dir"
  fi
}

# ---------------------------------------------------------------- git / releases

# Human-friendly ref of a checkout: its tag, else the short commit.
git_ref() {
  local dir="$1"
  git -C "$dir" describe --tags --always 2>/dev/null || echo "?"
}

# Full commit hash of a checkout (short form with git_head <dir> 12).
git_head() {
  local dir="$1" length="${2:-40}"
  git -C "$dir" rev-parse HEAD 2>/dev/null | cut -c "1-$length"
}

# Commit of the remote default branch: git_remote_head <dir> [length].
git_remote_head() {
  local dir="$1" length="${2:-40}"
  git -C "$dir" ls-remote origin HEAD 2>/dev/null | cut -c "1-$length"
}

# Newest stable release tag of a GitHub repo (empty when offline).
latest_github_release() {
  local repo="$1"
  upd_gh_latest "$repo"
}

# ---------------------------------------------------------------- targets / examples

# First column of sdk_targets, without header or note lines.
target_names() {
  sdk_targets | awk '$1 ~ /^[A-Za-z0-9][A-Za-z0-9_.@\/-]*$/ && $1 != "TARGET" {print $1}'
}

# Fail unless <target> is listed by sdk_targets.
require_target() {
  local target="$1"
  if target_names | grep -qxF -- "$target"; then
    return 0
  fi
  echo "unknown target '$target' (see: mcu targets ${SDK:-<sdk>})" >&2
  return 1
}

# Output dir of one build: $MCU_OUT/<target>/<example with / turned into _>.
build_dir() {
  local target="$1" example="${2:-}"
  if [[ -z "$example" ]]; then
    printf '%s/%s\n' "$MCU_OUT" "$target"
  else
    printf '%s/%s/%s\n' "$MCU_OUT" "$target" "${example//\//_}"
  fi
}

# ---------------------------------------------------------------- builds

# Configure (once) and build a CMake project with Ninja: cmake_build <src> <build-dir> [extra cmake args...].
cmake_build() {
  local src="$1" dir="$2"
  shift 2
  if [[ ! -f "$dir/CMakeCache.txt" || $# -gt 0 ]]; then
    cmake -S "$src" -B "$dir" -G Ninja "$@" || return 1
  fi
  cmake --build "$dir" -j "$MCU_JOBS"
}

# Run make in a directory with the workspace job count: make_build <dir> [make args...].
make_build() {
  local dir="$1"
  shift
  make -C "$dir" -j"$MCU_JOBS" "$@"
}

# Copy the build products matching the globs from <src-dir> into <out-dir>: copy_artifacts <src> <out> <glob...>.
copy_artifacts() {
  local src="$1" out="$2" pattern file copied=0
  shift 2
  mkdir -p "$out"
  for pattern in "$@"; do
    for file in "$src"/$pattern; do
      [[ -f "$file" ]] || continue
      cp -p "$file" "$out/"
      copied=$((copied + 1))
    done
  done
  ((copied > 0)) || {
    echo "no artifacts matching '$*' in $src" >&2
    return 1
  }
}

# Firmware files under <dir> (depth 3) as "path<TAB>bytes", sorted by path: list_artifacts <dir> [glob...].
list_artifacts() {
  local dir="$1" pattern
  shift
  [[ -d "$dir" ]] || return 0
  [[ $# -gt 0 ]] || set -- '*.elf' '*.bin' '*.hex' '*.uf2' '*.img' '*.img.gz' '*.kdimg' '*.dfu'
  local -a name_args=()
  for pattern in "$@"; do
    [[ ${#name_args[@]} -gt 0 ]] && name_args+=(-o)
    name_args+=(-name "$pattern")
  done
  find "$dir" -maxdepth 3 \( -name CMakeFiles -o -name .git \) -prune -o -type f \( "${name_args[@]}" \) -printf '%p\t%s\n' \
    | sort
}

# ---------------------------------------------------------------- trash (never rm)

# Trash directory for a path: the per-volume .Trash-<uid> of its mount when it is not $HOME's,
# otherwise ~/.local/share/Trash. MCU_TRASH (or the older UPD_TRASH) overrides it.
trash_dir_for() {
  local path="$1" mount home_mount uid volume_trash
  if [[ -n "${MCU_TRASH:-${UPD_TRASH:-}}" ]]; then
    printf '%s\n' "${MCU_TRASH:-$UPD_TRASH}"
    return 0
  fi
  uid="$(id -u)"
  mount="$(stat -c %m -- "$path" 2>/dev/null || echo /)"
  home_mount="$(stat -c %m -- "$HOME" 2>/dev/null || echo /)"
  volume_trash="$mount/.Trash-$uid"
  if [[ "$mount" != "$home_mount" ]] && { [[ -d "$volume_trash" && -w "$volume_trash" ]] || mkdir -m 700 "$volume_trash" 2>/dev/null; }; then
    printf '%s\n' "$volume_trash"
  else
    printf '%s\n' "$HOME/.local/share/Trash"
  fi
}

# Move <path> to the Trash (freedesktop layout: files/ + info/*.trashinfo, so it can be restored).
# trash_move <path> [name-hint]; honors --dry-run; a missing path is not an error.
trash_move() {
  local path="$1" hint="${2:-}" trash name abs
  if [[ ! -e "$path" && ! -L "$path" ]]; then
    return 0
  fi
  abs="$(readlink -f -- "$path")"
  if [[ "${MCU_DRY_RUN:-0}" == 1 ]]; then
    echo "would move $abs to the Trash"
    return 0
  fi
  trash="$(trash_dir_for "$abs")"
  [[ -n "$hint" ]] || hint="$(basename "$abs")"
  name="${hint//\//_}-$(date +%Y%m%d-%H%M%S)-$$-$RANDOM"
  mkdir -p "$trash/files" "$trash/info"
  printf '[Trash Info]\nPath=%s\nDeletionDate=%s\n' "$abs" "$(date +%Y-%m-%dT%H:%M:%S)" >"$trash/info/$name.trashinfo"
  mv -- "$abs" "$trash/files/$name"
  echo "moved $abs to the Trash ($trash/files/$name)"
}
