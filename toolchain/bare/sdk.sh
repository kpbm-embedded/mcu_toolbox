# shellcheck shell=bash disable=SC2034,SC2153
# bare plugin (contract v2, see ../PLUGIN.md): bare-metal Cortex-M and RISC-V without a vendor SDK. Shared CMake toolchain
# files (cmake/), chip layers with startup code and linker scripts (chips/<chip>/), and a runner that checks firmware
# in qemu (mps2-an385, riscv virt) or Renode (STM32F411/H743/F103, nRF52840, CH32V003): run/firmware.sh.
SDK_API=2
SDK_DESC="Bare metal Cortex-M/RISC-V: shared CMake toolchain files, chip startup + linker scripts, qemu/Renode test runs"
SDK_SELFCHECK="mps2-an385 hello-chips"
SDK_DEFAULT_EXAMPLE=hello-chips
SDK_FLASH_TOOLS="st-flash (STM32), probe-rs (nRF52840), wlink (CH32V003); qemu/virt chips run in the emulator instead"
BARE_EXAMPLES="$(cd -P "$SDK_DIR/../../examples/bare" 2>/dev/null && pwd -P)"

# shellcheck source=/dev/null  # this SDK's env.sh, resolved at run time
sdk_env() { source "$SDK_DIR/env.sh"; }

sdk_doctor() {
  local rc=0
  doctor_tool arm-none-eabi-gcc arm-none-eabi-gcc --version || rc=1
  doctor_tool riscv-none-elf-gcc riscv-none-elf-gcc --version || rc=1
  doctor_tool cmake cmake --version || rc=1
  doctor_tool ninja ninja --version || rc=1
  doctor_tool qemu-system-arm qemu-system-arm --version || rc=1
  doctor_tool qemu-system-riscv32 qemu-system-riscv32 --version || rc=1
  doctor_path renode "$SDK_DIR/renode/renode" "Renode (portable)" || rc=1
  return $rc
}

sdk_versions() {
  print_version arm-none-eabi-gcc "$(arm-none-eabi-gcc -dumpfullversion)"
  print_version riscv-none-elf-gcc "$(riscv-none-elf-gcc -dumpfullversion)"
  print_version qemu-arm "$(qemu-system-arm --version | head -1 | awk '{print $NF}')"
  print_version qemu-riscv "$(qemu-system-riscv32 --version | head -1 | awk '{print $NF}')"
  print_version renode "$(basename "$(readlink -f "$SDK_DIR/renode")" | sed 's/renode-//')"
}

sdk_targets() {
  local c
  for c in mps2-an385 virt-rv32 stm32f411 stm32h743 stm32f103 nrf52840 ch32v003; do
    printf '%-12s %s\n' "$c" "$(head -1 "$SDK_DIR/chips/$c/chip.cmake" | sed 's/^# //')"
  done
}

sdk_examples() {
  local d
  for d in "$BARE_EXAMPLES"/*/; do
    [[ -f $d/example.toml ]] && printf '%-22s %s\n' "$(basename "$d")" "$(sed -n 's/^description *= *"\(.*\)"/\1/p' "$d/example.toml")"
  done
  return 0
}

# Build <example> (a name under examples/bare, or a project path) for chip <target>.
sdk_build() {
  local target="$1" example="$2" src out
  shift 2
  require_target "$target" || return 1
  if [[ -d $example ]]; then src="$example"; else src="$BARE_EXAMPLES/$example"; fi
  [[ -f $src/CMakeLists.txt ]] || { echo "unknown example '$example' (mcu examples bare)" >&2; return 1; }
  out="$(build_dir "$target" "$(basename "$example")")"
  cmake_build "$src" "$out" -DCHIP="$target" "$@"
}

sdk_artifacts() { find "$(build_dir "$1" "$2")" -maxdepth 1 -name '*.elf' -o -maxdepth 1 -name '*.bin' | sort; }

# Flash needs a board (none attached here): the command per chip, run as is.
sdk_flash() {
  local target="$1" example="$2" elf
  elf="$(find "$(build_dir "$target" "$example")" -maxdepth 1 -name '*.elf' | head -1)"
  [[ -n $elf ]] || { echo "no build; run: mcu build bare $target $example" >&2; return 1; }
  case "$target" in
    stm32f411 | stm32h743 | stm32f103) st-flash --reset write "${elf%.elf}.bin" 0x08000000 ;;
    nrf52840) probe-rs download --chip nRF52840_xxAA "$elf" ;;
    ch32v003) wlink flash "${elf%.elf}.bin" ;;
    *) echo "$target is emulated: $SDK_DIR/run/firmware.sh $elf" >&2; return 1 ;;
  esac
}

sdk_monitor() { python3 -m serial.tools.miniterm "${1:-/dev/ttyUSB0}" "${2:-115200}"; }

sdk_info() {
  echo "runner      $SDK_DIR/run/firmware.sh <elf> (qemu / Renode, checks RESULT: PASS)"
  echo "cmake       $SDK_DIR/cmake/{arm-none-eabi,riscv-none-elf}.cmake, bare_chip.cmake + bare.cmake"
}
