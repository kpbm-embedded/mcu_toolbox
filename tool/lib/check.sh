# shellcheck shell=bash
# check.sh - health checks: doctor, selfcheck (strict-PATH build), libcheck (host binary libraries) and the
# host/sysbin directory of allowed OS utilities that selfcheck runs with.

# Allowed OS utility names from host/sysbin.list (several per line, # comments).
sysbin_names() {
  sed 's/#.*//' "$MCU_ROOT/host/sysbin.list" | tr -s ' \t' '\n' | grep -v '^$'
}

# Strict-mode PATH: symlinks to the OS base utilities named in host/sysbin.list, nothing else.
sysbin_refresh() {
  local dir="$MCU_ROOT/host/sysbin" name src
  mkdir -p "$dir"
  for name in $(sysbin_names); do
    [[ -e "$dir/$name" || -L "$dir/$name" ]] && continue
    # type -P: the file on disk even for names that are also shell builtins (printf, [, test, kill)
    src="$(PATH=/usr/bin:/bin type -P "$name" 2>/dev/null || true)"
    if [[ "$src" == /* ]]; then
      ln -s "$src" "$dir/$name"
    fi
  done
}

# Run plugin function <fn> for one SDK or for all of them (continuing past failures, with a header per SDK).
each_sdk_call() {
  local fn="$1" which="${2:-all}" sdk rc=0
  if [[ "$which" != all ]]; then
    plugin_call "$which" "$fn"
    return
  fi
  for sdk in $(sdk_names); do
    plugin_has "$sdk" "$fn" || continue
    printf '=== %s\n' "$sdk"
    plugin_call "$sdk" "$fn" || {
      log_error "$sdk: $fn failed"
      rc=1
    }
  done
  return "$rc"
}

# doctor [sdk|all]: required tools present, with versions.
cmd_doctor() {
  [[ $# -le 1 ]] || die_usage doctor "too many arguments"
  [[ "${1:-all}" == all ]] || require_sdk "$1"
  each_sdk_call sdk_doctor "${1:-all}"
}

# What selfcheck runs for <sdk>: "build <target> [example]", "fn" (sdk_selfcheck) or "" (skip).
selfcheck_spec() {
  local sdk="$1" spec
  spec="$(plugin_meta "$sdk" selfcheck)"
  if [[ -n "$spec" ]]; then
    printf 'build %s\n' "$spec"
  elif plugin_has "$sdk" sdk_selfcheck; then
    printf 'fn\n'
  fi
}

# Selfcheck one SDK: build its SDK_SELFCHECK target with PATH = host/sysbin only, log to _selfcheck/<sdk>.log.
selfcheck_one() {
  local sdk="$1" spec log reason rc=0
  local -a sub
  spec="$(selfcheck_spec "$sdk")"
  if [[ -z "$spec" ]]; then
    reason="$(plugin_meta "$sdk" no_selfcheck)"
    printf 'SKIP %s: %s\n' "$sdk" "${reason:-plugin sets no SDK_SELFCHECK}"
    return 0
  fi
  if [[ "$spec" == fn ]]; then
    sub=(_selfcheck "$sdk")
  else
    read -r -a sub <<<"$spec"
    sub=(build "$sdk" "${sub[@]:1}")
  fi
  if is_dry_run; then
    echo "would run (strict PATH): mcu ${sub[*]}"
    return 0
  fi
  log="$MCU_OUT_BASE/_selfcheck/$sdk.log"
  mkdir -p "$(dirname "$log")"
  env -i HOME="$HOME" USER="${USER:-}" TERM="${TERM:-dumb}" LANG=C.UTF-8 MCU_STRICT=1 MCU_COLOR=never \
    PATH="$MCU_ROOT/host/sysbin" MCU_OUT_BASE="$MCU_OUT_BASE/_selfcheck" MCU_JOBS="$MCU_JOBS" \
    bash "$MCU_ROOT/mcu" "${sub[@]}" >"$log" 2>&1 || rc=$?
  selfcheck_record "$sdk" "$rc"
  if ((rc == 0)); then
    printf '%s   %s (%s)\n' "$(paint 1 green OK)" "$sdk" "${spec#build }"
  else
    printf '%s %s (%s): see %s\n' "$(paint 1 red FAIL)" "$sdk" "${spec#build }" "$log"
    tail -5 "$log" | sed 's/^/     /'
    return 1
  fi
}

# Remember the last selfcheck result of <sdk> (read by `mcu status`).
selfcheck_record() {
  local sdk="$1" rc="$2"
  printf '%s\t%s\n' "$(date +%s)" "$( ((rc == 0)) && echo OK || echo FAIL)" | write_atomic "$(state_dir "$sdk")/last-selfcheck"
}

# selfcheck [sdk|all]
cmd_selfcheck() {
  local sdk rc=0
  [[ $# -le 1 ]] || die_usage selfcheck "too many arguments"
  sysbin_refresh
  for sdk in $(sdk_selection "${1:-all}"); do
    selfcheck_one "$sdk" || rc=1
  done
  return "$rc"
}

# Hidden: run sdk_selfcheck of one SDK (what selfcheck_one starts under the strict PATH).
cmd__selfcheck() {
  plugin_call "${1:?sdk}" sdk_selfcheck
}

# Run host/libcheck.py with the workspace CPython.
libcheck_run() {
  "$MCU_ROOT/host/python/cpython-3.12-linux-x86_64-gnu/bin/python3" -I "$MCU_ROOT/host/libcheck.py" "$@"
}

# libcheck [sdk|all]: host binaries that load non-glibc libraries from the OS.
cmd_libcheck() {
  [[ $# -le 1 ]] || die_usage libcheck "too many arguments"
  if [[ "${1:-all}" == all ]]; then
    libcheck_run "$MCU_ROOT"
  else
    require_sdk "$1"
    libcheck_run "$MCU_ROOT/$1"
  fi
}

# ---------------------------------------------------------------- check-clean

# Git checkouts inside the SDK dir (following its links, e.g. k230/sdk; west/repo projects and submodules included).
clean_git_repos() {
  local sdk="$1"
  find -L "$MCU_ROOT/$sdk" \( -name .venv -o -name .uv-cache -o -name __pycache__ -o -name node_modules \) -prune \
    -o -name .git -print 2>/dev/null | sed 's#/\.git$##' | sort -u
}

# Changes in one checkout: tracked modifications and untracked, non-ignored files (submodules are checked on their own).
clean_repo_changes() {
  local repo="$1"
  git -C "$repo" status --porcelain=v1 --ignore-submodules=all --untracked-files=normal 2>/dev/null
}

# Report one SDK; returns 1 when any of its checkouts has changes.
check_clean_sdk() {
  local sdk="$1" repo changes count=0 dirty=0
  while IFS= read -r repo; do
    [[ -n "$repo" ]] || continue
    count=$((count + 1))
    changes="$(clean_repo_changes "$repo")"
    [[ -n "$changes" ]] || continue
    dirty=$((dirty + 1))
    printf '%s %s %s: %d modified, %d untracked\n' "$(paint 1 red DIRTY)" "$sdk" "${repo#"$MCU_ROOT"/}" \
      "$(grep -vc '^??' <<<"$changes" || true)" "$(grep -c '^??' <<<"$changes" || true)"
    head -10 <<<"$changes" | sed 's/^/      /'
  done < <(clean_git_repos "$sdk")
  if ((dirty == 0)); then
    printf '%s %s (%d git checkouts)\n' "$(paint 1 green CLEAN)" "$sdk" "$count"
    return 0
  fi
  return 1
}

# check-clean [sdk|all]: builds must never change SDK code/config; list modified or untracked files in SDK checkouts.
cmd_check_clean() {
  local sdk rc=0
  [[ $# -le 1 ]] || die_usage check-clean "too many arguments"
  for sdk in $(sdk_selection "${1:-all}"); do
    check_clean_sdk "$sdk" || rc=1
  done
  return "$rc"
}
