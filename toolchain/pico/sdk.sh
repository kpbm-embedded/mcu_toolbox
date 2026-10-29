# pico plugin (see ../PLUGIN.md)
SDK_SELFCHECK="pico blink"
SDK_DESC="Raspberry Pi Pico SDK + pico-examples + picotool (RP2040/RP2350)"
sdk_env() { source "$SDK_DIR/env.sh"; }
_pico_map() { # target -> board platform
  case "$1" in
    pico) echo "pico";; pico2) echo "pico2";; pico2-riscv) echo "pico2 rp2350-riscv";;
    zero|rp2040-zero) echo "waveshare_rp2040_zero";; rp2350-zero) echo "waveshare_rp2350_zero";;
    rp2350-zero-riscv) echo "waveshare_rp2350_zero rp2350-riscv";;
    *) return 1;; esac; }
_pico_ex_target() { case "$1" in blink) echo blink;; hello_usb|hello_world/usb) echo hello_usb;; *) basename "$1";; esac; }
sdk_doctor() {
  local rc=0 t
  for t in cmake ninja arm-none-eabi-gcc picotool riscv32-pico-elf-gcc; do
    if command -v $t >/dev/null; then echo "ok $t $( ($t version 2>&1 | grep -q . && $t version 2>&1 || $t --version 2>&1) | head -n1)"; else echo "MISSING $t"; rc=1; fi; done
  [ -f "$PICO_SDK_PATH/pico_sdk_init.cmake" ] && echo "ok pico-sdk $(git -C "$PICO_SDK_PATH" describe --tags 2>/dev/null)" || { echo "MISSING pico-sdk"; rc=1; }
  [ -d "$PICO_EXAMPLES_PATH" ] && echo "ok pico-examples" || { echo "MISSING pico-examples"; rc=1; }
  return $rc; }
sdk_versions() {
  echo "pico-sdk      $(git -C "$PICO_SDK_PATH" describe --tags)"
  echo "pico-examples $(git -C "$PICO_EXAMPLES_PATH" describe --tags)"
  echo "picotool      $(picotool version)"
  echo "arm gcc       $(arm-none-eabi-gcc -dumpfullversion) (Arm GNU 15.2.rel1, local)"
  echo "riscv gcc     $(riscv32-pico-elf-gcc -dumpfullversion) (pico-sdk-tools)"; }
sdk_targets() { cat <<'T'
pico               Raspberry Pi Pico (RP2040)
zero               Waveshare RP2040-Zero (also SuperMini/Core-A, same chip)
pico2              Pico 2 (RP2350 ARM)
pico2-riscv        Pico 2 (RP2350 Hazard3 RISC-V)
rp2350-zero        Waveshare RP2350-Zero
rp2350-zero-riscv  Waveshare RP2350-Zero, RISC-V
T
}
sdk_examples() { echo "blink (default)"; echo "hello_usb"; echo "any target name/path in pico-examples (e.g. hello_world/usb is 'hello_usb')"; }
sdk_build() {
  local t=${1:?target} ex=${2:-blink}
  shift $(( $# >= 2 ? 2 : $# ))
  [ "${1:-}" = "--" ] && shift
  local m b p; m=$(_pico_map "$t") || { echo "unknown target $t"; return 1; }
  set -- $m; b=$1; p=${2:-}
  local d="$MCU_OUT/$t" extra=(); mkdir -p "$MCU_OUT"
  [ -n "$p" ] && extra+=(-DPICO_PLATFORM=$p)
  [ "$p" != rp2350-riscv ] && extra+=(-DPICO_TOOLCHAIN_PATH=$PICO_TOOLCHAIN_PATH)
  [ "$p" = rp2350-riscv ] && extra+=(-DPICO_TOOLCHAIN_PATH=$PICO_TOOLCHAIN_PATH_RISCV)
  cmake -G Ninja -S "$PICO_EXAMPLES_PATH" -B "$d" -DPICO_BOARD=$b "${extra[@]}" -Dpicotool_DIR="$picotool_DIR" >"$MCU_OUT/$t.cfg.log" 2>&1 || { echo "configure failed: $MCU_OUT/$t.cfg.log"; return 1; }
  local tgt; tgt=$(_pico_ex_target "$ex")
  cmake --build "$d" -j"$MCU_JOBS" --target "$tgt" || return 1
  find "$d" -name "$tgt.uf2" -o -name "$tgt.elf" | xargs ls -l; }
_pico_uf2() { find "$MCU_OUT/$1" -name "$(_pico_ex_target "${2:-blink}").uf2" | head -n1; }
sdk_flash() {
  local t=${1:?target} ex=${2:-blink} u; u=$(_pico_uf2 "$t" "$ex"); [ -n "$u" ] || { echo "no build; run build first"; return 1; }
  echo "flashing $u (board in BOOTSEL or running with pico_stdio_usb reset)"
  picotool load -x "$u" -f || echo "picotool failed: hold BOOTSEL, plug in, then copy $u to the RPI-RP2/RP2350 drive"; }
sdk_monitor() { python3 -m serial.tools.miniterm "${1:-/dev/ttyACM0}" "${2:-115200}" 2>/dev/null || echo "need pyserial; try: picocom -b ${2:-115200} ${1:-/dev/ttyACM0}"; }
sdk_clean() { local d="$MCU_OUT/${1:?target}"; [ -d "$d" ] && mv "$d" "$HOME/.local/share/Trash/files/pico-$1-$(date +%s)" && echo "moved $d to Trash"; }
_p_do() { local new="$1"
  upd_git_rec "${PICO_SDK_PATH#"$SDK_DIR"/}"; upd_git_rec "${PICO_EXAMPLES_PATH#"$SDK_DIR"/}"; upd_git_rec src-picotool
  git -C "$PICO_SDK_PATH" fetch -q --depth 1 origin tag "$new" && git -C "$PICO_SDK_PATH" checkout -q "$new" && git -C "$PICO_SDK_PATH" submodule update --init --recursive --depth 1 || return 1
  git -C "$PICO_EXAMPLES_PATH" fetch -q --depth 1 origin tag "sdk-$new" && git -C "$PICO_EXAMPLES_PATH" checkout -q "sdk-$new" || return 1
  git -C "$SDK_DIR/src-picotool" fetch -q --depth 1 origin tag "$new" && git -C "$SDK_DIR/src-picotool" checkout -q "$new" || return 1
  cp -p "$SDK_DIR/picotool/picotool" "$SDK_DIR/picotool/picotool.$(date +%Y%m%d-%H%M%S).bak" 2>/dev/null && upd_rec file picotool/picotool "$(ls -t "$SDK_DIR"/picotool/picotool.*.bak | head -1)"
  cmake -S "$SDK_DIR/src-picotool" -B "$MCU_OUT/picotool" -DCMAKE_INSTALL_PREFIX="$SDK_DIR" -DPICOTOOL_FLAT_INSTALL=1 -DCMAKE_BUILD_TYPE=Release >/dev/null &&
  cmake --build "$MCU_OUT/picotool" -j"$MCU_JOBS" >/dev/null && cmake --install "$MCU_OUT/picotool" >/dev/null; }
sdk_update() {
  UPD_RC=0
  local cur new; cur=$(git -C "$PICO_SDK_PATH" describe --tags)
  new=${1:-$(upd_git_newest_tag https://github.com/raspberrypi/pico-sdk '^[0-9]+\.[0-9]+\.[0-9]+$')}
  upd_step pico-sdk "$cur" "$new" _p_do "$new"
  echo "  (RISC-V toolchain riscv-toolchain-16 pinned; arm gcc is shared via tools/)"
  upd_dry_hint
  return "$UPD_RC"
}
