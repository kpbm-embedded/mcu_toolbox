# shellcheck shell=bash
# dev.sh - developer commands for the mcu scripts themselves: devtools, lint, fmt, test.
#
# Tools are pinned here (version + sha256) and installed inside the workspace:
#   host/devtools/<tool>-<version>/ + stable link host/devtools/<tool>, command in host/bin
#   tests/bats-core-<version>/ + stable link tests/bats
# "Owned" files (the framework and v2 plugins) must be lint/fmt clean; v1 plugin files are reported as pending.

# name|version|url|sha256 (GitHub release digests; bats-core publishes none, so its hash is pinned from the first verified fetch)
DEVTOOLS_MANIFEST='shellcheck|0.11.0|https://github.com/koalaman/shellcheck/releases/download/v0.11.0/shellcheck-v0.11.0.linux.x86_64.tar.xz|8c3be12b05d5c177a04c29e3c78ce89ac86f1595681cab149b65b97c4e227198
shfmt|3.14.1|https://github.com/mvdan/sh/releases/download/v3.14.1/shfmt_v3.14.1_linux_amd64|76e77641faa025814b77f153b29796b8e6fa2fca03e0c76a691608b86c7ea7bf
bats|1.14.0|https://github.com/bats-core/bats-core/archive/refs/tags/v1.14.0.tar.gz|bb537b70b15b732f6d8827dd6578e3d8ce166636ce1f18ea9a074184fcce9177'

SHFMT_STYLE=(-i 2 -ci -bn)

# Path of an installed dev tool's command (empty when missing).
devtool_path() {
  local tool="$1" path
  case "$tool" in
    bats) path="$MCU_ROOT/tests/bats/bin/bats" ;;
    *) path="$MCU_ROOT/host/bin/$tool" ;;
  esac
  [[ -x "$path" ]] && printf '%s\n' "$path"
  return 0
}

# Stop with a hint unless <tool> is installed.
require_devtool() {
  local tool="$1"
  [[ -n "$(devtool_path "$tool")" ]] || die "$tool is not installed; run: mcu devtools install"
}

# Download, verify and unpack one manifest entry side by side, then flip its stable link.
devtool_install_one() {
  local tool="$1" version="$2" url="$3" sha="$4" archive dest
  archive="$MCU_ROOT/host/dl/$(basename "$url")"
  [[ "$tool" == bats ]] && archive="$MCU_ROOT/host/dl/bats-core-$version.tar.gz"
  devtool_fetch "$url" "$archive" "$sha" || return 1
  case "$tool" in
    shellcheck)
      dest="$(devtool_dest shellcheck "$version")"
      devtool_unpack "$archive" "$dest" 1
      ln -sfn "../devtools/shellcheck/shellcheck" "$MCU_ROOT/host/bin/shellcheck.tmp$$"
      mv -T "$MCU_ROOT/host/bin/shellcheck.tmp$$" "$MCU_ROOT/host/bin/shellcheck"
      ;;
    shfmt)
      dest="$(devtool_dest shfmt "$version")"
      mkdir -p "$dest"
      install -m 755 "$archive" "$dest/shfmt"
      ln -sfn "../devtools/shfmt/shfmt" "$MCU_ROOT/host/bin/shfmt.tmp$$"
      mv -T "$MCU_ROOT/host/bin/shfmt.tmp$$" "$MCU_ROOT/host/bin/shfmt"
      ;;
    bats)
      dest="$(devtool_dest bats "$version")"
      devtool_unpack "$archive" "$dest" 1
      ln -sfn "bats-core-$version" "$MCU_ROOT/tests/bats.tmp$$"
      mv -T "$MCU_ROOT/tests/bats.tmp$$" "$MCU_ROOT/tests/bats"
      return 0
      ;;
  esac
  ln -sfn "$tool-$version" "$MCU_ROOT/host/devtools/$tool.tmp$$"
  mv -T "$MCU_ROOT/host/devtools/$tool.tmp$$" "$MCU_ROOT/host/devtools/$tool"
}

# Download <url> to <archive> once and verify its pinned sha256 (upd_fetch, recorded in host/.update-sha256).
devtool_fetch() {
  (
    export SDK=host SDK_DIR="$MCU_ROOT/host"
    # shellcheck source=lib/update.sh
    source "$MCU_ROOT/lib/update.sh"
    upd_fetch "$1" "$2" "hex:$3"
  )
}

# Extract into <dest>.part and rename (an existing dest is kept).
devtool_unpack() {
  local archive="$1" dest="$2" strip="$3"
  [[ -d "$dest" ]] && return 0
  mkdir -p "$dest.part"
  tar -xf "$archive" -C "$dest.part" --strip-components="$strip"
  mv -T "$dest.part" "$dest"
}

# Versioned install dir of a dev tool.
devtool_dest() {
  local tool="$1" version="$2"
  if [[ "$tool" == bats ]]; then
    printf '%s/tests/bats-core-%s\n' "$MCU_ROOT" "$version"
  else
    printf '%s/host/devtools/%s-%s\n' "$MCU_ROOT" "$tool" "$version"
  fi
}

# One status line per pinned dev tool.
devtools_status() {
  local tool version url sha path
  while IFS='|' read -r tool version url sha; do
    path="$(devtool_path "$tool")"
    if [[ -n "$path" ]]; then
      printf 'ok      %-11s %s  (%s)\n' "$tool" "$version" "${path#"$MCU_ROOT"/}"
    else
      printf 'MISSING %-11s %s  (mcu devtools install)\n' "$tool" "$version"
    fi
  done <<<"$DEVTOOLS_MANIFEST"
}

# devtools [install]: show or install the pinned shellcheck, shfmt and bats-core.
cmd_devtools() {
  local tool version url sha
  case "${1:-status}" in
    status) devtools_status ;;
    install)
      mkdir -p "$MCU_ROOT/host/devtools" "$MCU_ROOT/host/dl"
      while IFS='|' read -r tool version url sha; do
        [[ -n "$(devtool_path "$tool")" && -d "$(devtool_dest "$tool" "$version")" ]] && continue
        echo "--- $tool $version"
        if is_dry_run; then
          echo "would fetch $url"
          continue
        fi
        devtool_install_one "$tool" "$version" "$url" "$sha" || die "$tool install failed"
      done <<<"$DEVTOOLS_MANIFEST"
      devtools_status
      ;;
    *) die_usage devtools "unknown action '$1' (status|install)" ;;
  esac
}

# ---------------------------------------------------------------- file sets

# Shell files of the framework (always lint/fmt clean).
dev_framework_files() {
  printf '%s\n' "$MCU_ROOT/mcu" "$MCU_ROOT"/lib/*.sh "$MCU_ROOT/completion/mcu.bash" \
    "$MCU_ROOT/host/update-lib.sh" "$MCU_ROOT/host/patch-rpath.sh" \
    "$MCU_ROOT"/tests/*.bats "$MCU_ROOT"/tests/*.bash "$MCU_ROOT"/tests/fixtures/*/sdk.sh "$MCU_ROOT"/tests/fixtures/*/env.sh \
    | while IFS= read -r file; do
      if [[ -f "$file" ]]; then printf '%s\n' "$file"; fi
    done
}

# Plugin shell files: owned=1 gives sdk.sh/env.sh of v2 plugins; owned=0 the rest (v1 plugins and helper
# scripts kept for them, e.g. stm32/sdk-common.sh used by freertos), all pending migration.
dev_plugin_files() {
  local want_owned="$1" sdk file owned
  for sdk in $(sdk_names); do
    for file in "$MCU_ROOT/$sdk"/*.sh; do
      [[ -f "$file" ]] || continue
      [[ "$sdk" == host && ("$file" == */update-lib.sh || "$file" == */patch-rpath.sh) ]] && continue
      owned=0
      if plugin_is_v2 "$sdk" && [[ "$file" == */sdk.sh || "$file" == */env.sh ]]; then owned=1; fi
      if [[ "$owned" == "$want_owned" ]]; then printf '%s\n' "$file"; fi
    done
  done
}

# The files a dev command works on: owned (framework + v2) or, with --all, everything.
dev_owned_files() {
  dev_framework_files
  dev_plugin_files 1
}

# ---------------------------------------------------------------- lint / fmt / test

# Run shellcheck on files; prints findings and returns their count via LINT_COUNT.
lint_files() {
  local shellcheck file out
  shellcheck="$(devtool_path shellcheck)"
  LINT_COUNT=0
  for file in "$@"; do
    out="$(cd "$MCU_ROOT" && "$shellcheck" -x -f gcc "$file" 2>&1)" || true
    [[ -n "$out" ]] || continue
    printf '%s\n' "$out"
    LINT_COUNT=$((LINT_COUNT + $(grep -c ': \(error\|warning\|note\|style\):' <<<"$out")))
  done
}

# lint [--all]: shellcheck over owned files; v1 plugin findings are pending (count only with --all).
cmd_lint() {
  local all=0 owned_count pending_count
  local -a owned pending
  [[ "${1:-}" == --all ]] && all=1
  require_devtool shellcheck
  mapfile -t owned < <(dev_owned_files)
  mapfile -t pending < <(dev_plugin_files 0)
  lint_files "${owned[@]}"
  owned_count="$LINT_COUNT"
  if ((all)); then
    lint_files "${pending[@]}"
  else
    lint_files "${pending[@]}" >/dev/null
  fi
  pending_count="$LINT_COUNT"
  printf 'lint: %d findings in %d owned files (framework + v2 plugins)\n' "$owned_count" "${#owned[@]}"
  printf 'lint: %d findings in %d v1 plugin files pending migration%s\n' "$pending_count" "${#pending[@]}" \
    "$( ((all)) || echo ' (details: mcu lint --all)')"
  ((owned_count == 0)) || return 1
  ((all == 0 || pending_count == 0))
}

# Rewrite one file with shfmt atomically (formatted copy next to it, then mv over the original).
fmt_rewrite() {
  local shfmt="$1" file="$2"
  "$shfmt" "${SHFMT_STYLE[@]}" "$file" >"$file.fmt.$$"
  chmod --reference="$file" "$file.fmt.$$"
  mv -f "$file.fmt.$$" "$file"
}

# fmt [--check] [--all]: shfmt with the workspace style (-i 2 -ci -bn).
cmd_fmt() {
  local check=0 all=0 shfmt file listing rc=0
  local -a files pending
  while [[ $# -gt 0 ]]; do
    case "$1" in
      --check) check=1 ;;
      --all) all=1 ;;
      *) die_usage fmt "unknown argument '$1'" ;;
    esac
    shift
  done
  require_devtool shfmt
  shfmt="$(devtool_path shfmt)"
  mapfile -t files < <(dev_owned_files)
  ((all)) && mapfile -t -O "${#files[@]}" files < <(dev_plugin_files 0)
  # -l lists the files whose formatting differs (and reports parse errors on stderr)
  listing="$("$shfmt" "${SHFMT_STYLE[@]}" -l "${files[@]}")" || rc=1
  pending=()
  [[ -z "$listing" ]] || mapfile -t pending <<<"$listing"
  for file in "${pending[@]}"; do
    if ((check)); then
      echo "would reformat: ${file#"$MCU_ROOT"/}"
    else
      fmt_rewrite "$shfmt" "$file"
      echo "formatted: ${file#"$MCU_ROOT"/}"
    fi
  done
  printf 'fmt: %d of %d files %s\n' "${#pending[@]}" "${#files[@]}" "$( ((check)) && echo 'need formatting' || echo 'reformatted')"
  ((rc == 0)) || return 1
  ((check == 0 || ${#pending[@]} == 0))
}

# test [bats args...]: run tests/*.bats against this mcu.
cmd_test() {
  require_devtool bats
  MCU_UNDER_TEST="$MCU_ROOT/mcu" "$(devtool_path bats)" "$@" "$MCU_ROOT"/tests/*.bats
}
