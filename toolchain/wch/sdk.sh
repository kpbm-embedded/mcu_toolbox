# shellcheck shell=bash disable=SC2034,SC2153
# wch plugin (contract v2, see ../PLUGIN.md): WCH CH32V RISC-V with xPack riscv-none-elf-gcc,
# ch32fun (+ minichlink), openwch EVT trees, wlink and wchisp. SC2034/SC2153: SDK_* are read by ../mcu; MCU_* come from it.
#
# Versioned components live side by side behind stable links (toolchain/xpack-riscv-none-elf-gcc, wlink, wchisp);
# env.sh names only those links, so updates never edit it.

SDK_API=2
SDK_DESC="WCH CH32V RISC-V: xPack gcc + ch32fun/minichlink + openwch EVT, wlink, wchisp"
SDK_SELFCHECK="ch32v003 blink"
SDK_DEFAULT_EXAMPLE=blink
SDK_FLASH_TOOLS="minichlink (default, WCH-LinkE), wlink, wchisp (USB ISP)"

sdk_env() {
  # shellcheck source=wch/env.sh
  source "$SDK_DIR/env.sh"
}

# ---------------------------------------------------------------- doctor / versions

sdk_doctor() {
  local rc=0
  doctor_tool riscv-gcc riscv-none-elf-gcc || rc=1
  doctor_tool make make || rc=1
  doctor_path minichlink "$SDK_DIR/bin/minichlink" "$(git_head "$CH32FUN_REPO" 7)" || rc=1
  doctor_tool wlink wlink || rc=1
  doctor_tool wchisp wchisp || rc=1
  doctor_path "openwch/ch32v20x EVT" "$SDK_DIR/ch32v20x/EVT/EXAM/SRC/Ld/Link.ld" || rc=1
  return "$rc"
}

sdk_versions() {
  print_version xpack-gcc "$(tool_version riscv-none-elf-gcc --version)"
  print_version ch32fun "$(git_head "$CH32FUN_REPO")"
  print_version ch32v20x "$(git_head "$SDK_DIR/ch32v20x")"
  print_version ch32v307 "$(git_head "$SDK_DIR/ch32v307")"
  print_version wlink "$(tool_version wlink --version)"
  print_version wchisp "$(tool_version wchisp --version)"
}

# ---------------------------------------------------------------- targets / examples

sdk_targets() {
  cat <<'EOF'
ch32v003   CH32V003 (WeAct F4U6 core / EVT)  [ch32fun]
ch32v203   CH32V203 (WeAct C8T6)             [ch32fun, openwch EVT]
ch32v307   CH32V307VCT6 dev board (also V305)[ch32fun]
ch32h417   CH32H417                          [ch32fun examples_h41x]
EOF
}

sdk_examples() {
  echo "blink     ch32fun blink (all targets; h417 maps to 'regular')"
  echo "evt-gpio  openwch EVT GPIO_Toggle (ch32v203 only)"
  echo "<name>    any dir in ch32fun/examples*/ for the target"
}

# ch32fun example directory of <target> <example>.
wch_example_dir() {
  local target="$1" example="$2" base
  case "$target" in
    ch32v003) base=examples ;;
    ch32v203) base=examples_v20x ;;
    ch32v307) base=examples_v30x ;;
    ch32h417) base=examples_h41x ;;
    *)
      echo "unknown target $target" >&2
      return 1
      ;;
  esac
  [[ "$target" == ch32h417 && "$example" == blink ]] && example=regular
  echo "$CH32FUN_REPO/$base/$example"
}

# ch32fun TARGET_MCU name of <target>.
wch_mcu_name() {
  case "$1" in
    ch32v003) echo CH32V003 ;;
    ch32v203) echo CH32V203C8T6 ;;
    ch32v307) echo CH32V307 ;;
    ch32h417) echo CH32H417 ;;
  esac
}

# ---------------------------------------------------------------- build / flash / monitor

# openwch EVT GPIO_Toggle (ch32v203 only), built straight into <out>.
wch_build_evt_gpio() {
  local target="$1" out="$2"
  shift 2
  [[ "$target" == ch32v203 ]] || {
    echo "evt-gpio is ch32v203 only" >&2
    return 1
  }
  make_build "$SDK_DIR/evt-v203-gpio" BUILD="$out" "$@"
}

# Out-of-tree work copy of a ch32fun example under <work>: the example's tracked files are copied
# (mtimes kept, so rebuilds stay incremental) and the rest of the ch32fun repo is linked next to it, so the
# example's relative includes resolve while nothing is ever written into the repo. Prints the example's work dir.
wch_prepare_worktree() {
  local dir="$1" work="$2" entry relative
  relative="${dir#"$CH32FUN_REPO"/}"
  mkdir -p "$work"
  for entry in "$CH32FUN_REPO"/*; do
    [[ "$(basename "$entry")" == examples* ]] && continue
    [[ -e "$work/$(basename "$entry")" ]] || ln -s "$entry" "$work/$(basename "$entry")"
  done
  (cd "$CH32FUN_REPO" && git ls-files -z -- "$relative" | xargs -0 -r cp -p --parents -t "$work") || return 1
  echo "$work/$relative"
}

# A ch32fun example, built out of tree in <out>/src, then its elf/bin/hex copied into <out>.
wch_build_ch32fun() {
  local target="$1" example="$2" out="$3" dir mcu work
  shift 3
  dir="$(wch_example_dir "$target" "$example")" || return 1
  [[ -d "$dir" ]] || {
    echo "no example dir $dir" >&2
    return 1
  }
  work="$(wch_prepare_worktree "$dir" "$out/src")" || return 1
  mcu="$(wch_mcu_name "$target")"
  make_build "$work" build TARGET_MCU="$mcu" TARGET_MCU_PACKAGE="$mcu" GENERATED_LD_FILE="$out/src/generated.ld" "$@" || return 1
  copy_artifacts "$work" "$out" "$example.elf" "$example.bin" "$example.hex" 2>/dev/null \
    || copy_artifacts "$work" "$out" '*.elf' '*.bin' '*.hex'
}

# sdk_build <target> <example> [extra make args]
sdk_build() {
  local target="$1" example="$2" out
  shift 2
  require_target "$target" || return 1
  out="$(build_dir "$target" "$example")"
  mkdir -p "$out"
  if [[ "$example" == evt-gpio ]]; then
    wch_build_evt_gpio "$target" "$out" "$@" || return 1
  else
    wch_build_ch32fun "$target" "$example" "$out" "$@" || return 1
  fi
  riscv-none-elf-size "$out"/*.elf
}

# sdk_artifacts <target> <example>: the copied elf/bin/hex at the top of the build dir (not the work copy in src/).
sdk_artifacts() {
  find "$(build_dir "$1" "$2")" -maxdepth 1 -type f \( -name '*.elf' -o -name '*.bin' -o -name '*.hex' \) | sort
}

# sdk_flash <target> <example> [minichlink|wlink|wchisp]
sdk_flash() {
  local target="$1" example="$2" tool="${3:-minichlink}" bin
  bin="$(sdk_artifacts "$target" "$example" | grep '\.bin$' | grep -v '_ext\.bin$' | head -1)"
  [[ -n "$bin" ]] || {
    echo "build first: mcu build wch $target $example" >&2
    return 1
  }
  case "$tool" in
    minichlink) minichlink -w "$bin" flash -b ;;
    wlink) wlink flash "${bin%.bin}.hex" ;;
    wchisp) wchisp flash "${bin%.bin}.hex" ;;
    *)
      echo "tool must be minichlink|wlink|wchisp" >&2
      return 1
      ;;
  esac
}

# sdk_monitor [port] [baud]: the WCH-Link VCP, or (no port) CH32V003 printf over SDI via minichlink -T.
sdk_monitor() {
  local port="${1:-}"
  if [[ -n "$port" ]]; then
    python3 -m serial.tools.miniterm "$port" "${2:-115200}"
  else
    minichlink -T
  fi
}

# Download cache of `mcu update` (cleaned by `mcu clean --cache wch`; versions already installed stay).
sdk_cache_dirs() {
  echo "$SDK_DIR/dl"
}

# ---------------------------------------------------------------- update

sdk_prune_families() {
  cat <<'EOF'
toolchain/xpack-riscv-none-elf-gcc|xpack-riscv-none-elf-gcc-[0-9]*
wlink|wlink-[0-9]*
wchisp|wchisp-[0-9]*
EOF
}

# Copy the <name> binary found anywhere in <archive> to <SDK_DIR>/<name>-<version>/<name>.
# The extraction dir is kept under $MCU_OUT (never deleted).
wch_unpack_single_binary() {
  local archive="$1" name="$2" version="$3" dest="$SDK_DIR/$2-$3" binary
  [[ -d "$dest" ]] && return 0
  mkdir -p "$dest.part" && tar -xf "$archive" -C "$dest.part" || return 1
  binary="$(find "$dest.part" -type f -name "$name" | head -1)"
  [[ -n "$binary" ]] || return 1
  mkdir -p "$dest" && cp -p "$binary" "$dest/$name" && chmod +x "$dest/$name" || return 1
  mkdir -p "$MCU_OUT"
  mv "$dest.part" "$MCU_OUT/$name-$version.extract-$(date +%s)"
}

# Install GitHub release <tag> of <repo> as <name>-<version>/<name> and flip the <name> link.
wch_install_github_tool() {
  local repo="$1" tag="$2" name="$3" asset_regex="$4" version="${2#v}" info url digest sha_url archive
  info="$(upd_gh_asset "$repo" "$tag" "$asset_regex")"
  IFS=$'\t' read -r url digest sha_url <<<"$info"
  [[ -n "$url" ]] || {
    echo "  no linux-x64 asset for $repo $tag" >&2
    return 1
  }
  archive="$SDK_DIR/dl/$name-$version-$(basename "$url")"
  upd_fetch "$url" "$archive" "${sha_url:+url:$sha_url}${sha_url:-${digest:+hex:$digest}}" || return 1
  wch_unpack_single_binary "$archive" "$name" "$version" || return 1
  upd_patch "$SDK_DIR/$name-$version"
  upd_flip "$SDK_DIR/$name" "$name-$version"
}

# Keep a timestamped copy of bin/minichlink and record it for rollback.
wch_backup_minichlink() {
  local backup
  backup="$SDK_DIR/bin/minichlink.$(date +%Y%m%d-%H%M%S).bak"
  cp -p "$SDK_DIR/bin/minichlink" "$backup" && upd_rec file bin/minichlink "$backup"
}

# Fast-forward git repo <dir>; when it is ch32fun, rebuild bin/minichlink from it.
wch_pull_repo() {
  local dir="$1"
  upd_git_rec "$dir"
  git -C "$SDK_DIR/$dir" pull -q --ff-only || return 1
  [[ "$dir" == ch32fun ]] || return 0
  wch_backup_minichlink || return 1
  make_build "$SDK_DIR/ch32fun/minichlink" && cp "$SDK_DIR/ch32fun/minichlink/minichlink" "$SDK_DIR/bin/minichlink"
}

# Update step for one tracked git repo (local HEAD vs remote HEAD).
wch_update_repo() {
  local dir="$1" local_head remote_head
  local_head="$(git_head "$SDK_DIR/$dir" 12)"
  remote_head="$(git_remote_head "$SDK_DIR/$dir" 12)"
  upd_step "git:$dir" "$local_head" "${remote_head:-$local_head}" wch_pull_repo "$dir"
}

# Update step for a ch32-rs release tool (wlink, wchisp).
wch_update_release_tool() {
  local name="$1" tag installed
  tag="$(latest_github_release "ch32-rs/$name")"
  installed="$(tool_version "$name" --version | awk '{print $2}')"
  upd_step "$name" "$installed" "${tag#v}" wch_install_github_tool "ch32-rs/$name" "$tag" "$name" "$name-v.*-linux-x64\\.tar\\.gz"
}

# Update step for the xPack RISC-V toolchain.
wch_update_toolchain() {
  local info tag version current
  info="$(upd_xpack_info xpack-dev-tools/riscv-none-elf-gcc-xpack)"
  IFS=$'\t' read -r tag version _ <<<"$info"
  current="$(readlink "$SDK_DIR/toolchain/xpack-riscv-none-elf-gcc" | sed 's/^xpack-riscv-none-elf-gcc-//')"
  upd_step xpack-riscv-gcc "$current" "$version" upd_install_xpack xpack-dev-tools/riscv-none-elf-gcc-xpack \
    "$SDK_DIR/toolchain" xpack-riscv-none-elf-gcc xpack-riscv-none-elf-gcc
}

sdk_update() {
  UPD_RC=0
  local repo
  wch_update_toolchain
  wch_update_release_tool wlink
  wch_update_release_tool wchisp
  for repo in ch32fun ch32v20x ch32v307; do
    wch_update_repo "$repo"
  done
  upd_dry_hint
  return "$UPD_RC"
}
