# bouffalo plugin (see ../PLUGIN.md)
SDK_SELFCHECK="bl616 helloworld"
SDK_DESC="Bouffalo SDK (BL616/BL702) + Xuantie RISC-V GCC + BLFlashCommand/bflb-mcu-tool"
sdk_env() { source "$SDK_DIR/env.sh"; }
_bl_map() { case "$1" in
  bl616) echo "bl616 bl616dk";; bl808|bl808-m0) echo "bl808 bl808dk m0";; bl808-d0) echo "bl808 bl808dk d0";; bl616cl) echo "bl616cl bl616cldk";; bl702) echo "bl702 bl702dk";; bl702l) echo "bl702l bl702ldk";;
  *) return 1;; esac; }
_bl_dir() { echo "$MCU_OUT/$1/${2:-helloworld}"; }
sdk_doctor() {
  local rc=0 t
  for t in riscv64-unknown-elf-gcc make cmake ninja bflb-mcu-tool; do
    if command -v $t >/dev/null; then echo "ok $t $( [ $t = bflb-mcu-tool ] && echo "$("$SDK_DIR/venv/bin/pip" list 2>/dev/null | awk "/bflb-mcu-tool/{print \$2}")" || $t --version 2>&1 | head -n1)"; else echo "MISSING $t"; rc=1; fi; done
  [ -x "$BL_SDK_BASE/tools/bflb_tools/bouffalo_flash_cube/BLFlashCommand-ubuntu" ] && echo "ok BLFlashCommand-ubuntu" || { echo "MISSING BLFlashCommand-ubuntu"; rc=1; }
  return $rc; }
sdk_versions() {
  echo "bouffalo_sdk $(git -C "$BL_SDK_BASE" describe --tags 2>/dev/null || cat "$BL_SDK_BASE/VERSION")"
  echo "toolchain    $(riscv64-unknown-elf-gcc --version | head -n1)"
  echo "bflb-mcu-tool $("$SDK_DIR/venv/bin/pip" list 2>/dev/null | awk '/bflb-mcu-tool/{print $2}')"; }
sdk_targets() { cat <<'T'
bl616    Sipeed M0s Dock (BL616) / Ai-M62 (BL616), board bl616dk
bl702    Sipeed M0sense (BL702), RV-Debugger Plus (BL702), board bl702dk
bl616cl  BL616CL, board bl616cldk
bl702l   BL702L, board bl702ldk
bl808    Sipeed M1s Dock / M1s module, M0 core (legacy SDK 2.2.0, board bl808dk)
bl808-d0 BL808 D0 (64-bit) core (legacy SDK)
T
}
sdk_examples() { (cd "$BL_SDK_BASE/examples" && ls -d */ | sed 's#/$##'); echo "(default: helloworld)"; }
sdk_build() {
  local t=${1:?target} ex=${2:-helloworld}
  shift $(( $# >= 2 ? 2 : $# )); [ "${1:-}" = "--" ] && shift
  local m; m=$(_bl_map "$t") || { echo "unknown target $t"; return 1; }; set -- $m "$@"; local chip=$1 board=$2 extra=""; shift 2
  local sdkb=$BL_SDK_BASE
  if [ "$chip" = bl808 ]; then sdkb=$SDK_DIR/bouffalo_sdk_legacy; extra="CPU_ID=$1"; shift; fi
  [ -d "$sdkb/examples/$ex" ] || { echo "no example $ex"; return 1; }
  local d; d=$(_bl_dir "$t" "$ex"); mkdir -p "$d"
  [ -d "$d/src" ] || cp -r "$sdkb/examples/$ex" "$d/src"   # out-of-tree copy: SDK tree stays clean
  (cd "$d/src" && BL_SDK_BASE=$sdkb make CHIP=$chip BOARD=$board $extra -j"$MCU_JOBS" "$@" >"$d/build.log" 2>&1) || { tail -n 20 "$d/build.log"; return 1; }
  ls -l "$d"/src/build/build_out/*.bin "$d"/src/build/build_out/*.elf 2>/dev/null | grep -vE 'boot2|efuse|partition|mfg'; }
sdk_flash() {
  local t=${1:?target} ex=${2:-helloworld} port=${3:-/dev/ttyACM0}
  local m; m=$(_bl_map "$t"); set -- $m; local d; d=$(_bl_dir "$t" "$ex")
  echo "Put the board in boot mode: hold BOOT, tap RESET (or plug in with BOOT held), then flash over $port"
  local sdkb=$BL_SDK_BASE extra=""; [ "$1" = bl808 ] && { sdkb=$SDK_DIR/bouffalo_sdk_legacy; extra="CPU_ID=$3"; }
  (cd "$d/src" && BL_SDK_BASE=$sdkb make flash CHIP=$1 BOARD=$2 $extra COMX="$port"); }
sdk_monitor() { "$SDK_DIR/venv/bin/python" -m serial.tools.miniterm "${1:-/dev/ttyACM0}" "${2:-2000000}"; }
sdk_clean() { local d; d=$(_bl_dir "${1:?target}" "${2:-helloworld}"); [ -d "$d" ] && mv "$d" "$HOME/.local/share/Trash/files/bl-$1-$(date +%s)" && echo "moved $d to Trash"; }
_b_do() { local new="$1"; upd_git_rec bouffalo_sdk; upd_rec pip venv/bin/python "bflb-mcu-tool==$(uv pip freeze -p "$SDK_DIR/venv/bin/python" | sed -n 's/^bflb-mcu-tool==//p')"
  git -C "$BL_SDK_BASE" fetch -q --depth 1 origin tag "$new" && git -C "$BL_SDK_BASE" checkout -q "$new" || return 1
  uv pip install -q -U -p "$SDK_DIR/venv/bin/python" bflb-mcu-tool pyserial; }
sdk_update() {
  UPD_RC=0
  local cur new; cur=$(git -C "$BL_SDK_BASE" describe --tags 2>/dev/null || echo unknown)
  new=${1:-$(upd_git_newest_tag https://github.com/bouffalolab/bouffalo_sdk '^v[0-9]+\.[0-9]+\.[0-9]+$')}
  upd_step bouffalo_sdk "$cur" "$new" _b_do "$new"
  echo "  (xuantie toolchain pinned v2.10.2; report only: https://github.com/bouffalolab/toolchain/releases/tag/default)"
  upd_dry_hint
  return "$UPD_RC"
}
