# shellcheck shell=bash disable=SC2034,SC2153
# stm32 plugin (contract v2, see ../PLUGIN.md): STM32Cube F4/F1/G4/H7 packs + CMake/Ninja HAL examples.
# The Arm toolchain and the flash tools come from ../tools (sourced by env.sh). SC2034/SC2153: SDK_* are read by ../mcu; MCU_* come from it.

SDK_API=2
SDK_DESC="STM32Cube (F4/F1/G4/H7) + HAL examples, flash via st-flash/openocd/pyocd/probe-rs/dfu"
SDK_SELFCHECK="blackpill-f411 blinky-f411"
SDK_DEFAULT_EXAMPLE=blinky-f411
SDK_FLASH_TOOLS="st-flash (default), openocd, pyocd, probe-rs, dfu"

STM32_FAMILIES=(F4 F1 G4 H7)
STM32_EXAMPLES="$SDK_DIR/examples"

sdk_env() {
  # shellcheck source=stm32/env.sh
  source "$SDK_DIR/env.sh"
}

# ---------------------------------------------------------------- doctor / versions

# Warn when arm-none-eabi-gcc is not the toolchain bundled in tools/toolchains.
stm32_check_bundled_gcc() {
  local gcc tc
  gcc="$(command -v arm-none-eabi-gcc || true)"
  tc="$(cd "$SDK_DIR/../tools/toolchains" 2>/dev/null && pwd -P)"   # resolved: symlinked trees still match
  [[ -n "$gcc" && -n "$tc" && "$(readlink -f "$gcc")" == "$tc"/* ]] || print_warn arm-none-eabi-gcc "not the bundled toolchain: ${gcc:-none}"
  return 0
}

sdk_doctor() {
  local rc=0 family tool
  doctor_tool arm-none-eabi-gcc arm-none-eabi-gcc || rc=1
  stm32_check_bundled_gcc
  for tool in cmake ninja st-flash openocd pyocd probe-rs; do
    doctor_tool "$tool" "$tool" || rc=1
  done
  has_tool dfu-util || print_warn dfu-util "not installed (optional; DFU flashing unavailable)"
  for family in "${STM32_FAMILIES[@]}"; do
    doctor_path "STM32Cube$family" "$SDK_DIR/STM32Cube$family/Drivers" "$(git_ref "$SDK_DIR/STM32Cube$family")" || rc=1
  done
  return "$rc"
}

sdk_versions() {
  local family
  for family in "${STM32_FAMILIES[@]}"; do
    print_version "STM32Cube$family" "$(git_ref "$SDK_DIR/STM32Cube$family")"
  done
  print_version arm-none-eabi-gcc "$(tool_version arm-none-eabi-gcc --version)"
  print_version cmake "$(tool_version cmake --version)"
}

# ---------------------------------------------------------------- targets / examples / build

sdk_targets() {
  echo "blackpill-f411  WeAct BlackPill STM32F411CEU6 (owned)"
}

sdk_examples() {
  local dir
  for dir in "$STM32_EXAMPLES"/*/; do
    [[ -f "$dir/CMakeLists.txt" ]] && basename "$dir"
  done
  return 0
}

# Fail unless <example> is one of sdk_examples.
stm32_require_example() {
  local example="$1"
  [[ -f "$STM32_EXAMPLES/$example/CMakeLists.txt" ]] && return 0
  echo "unknown example '$example' (have: $(sdk_examples | tr '\n' ' '))" >&2
  return 1
}

# sdk_build <target> <example> [extra cmake args]
sdk_build() {
  local target="$1" example="$2" out
  shift 2
  require_target "$target" || return 1
  stm32_require_example "$example" || return 1
  out="$(build_dir "$target" "$example")"
  cmake_build "$STM32_EXAMPLES/$example" "$out" "$@" || return 1
  [[ -f "$out/$example.bin" ]] || {
    echo "build produced no $example.bin in $out" >&2
    return 1
  }
}

# sdk_flash <target> <example> [st-flash|openocd|pyocd|probe-rs|dfu]
sdk_flash() {
  local target="$1" example="$2" probe="${3:-st-flash}" out
  require_target "$target" || return 1
  out="$(build_dir "$target" "$example")"
  [[ -f "$out/$example.bin" ]] || {
    echo "not built: mcu build $SDK $target $example" >&2
    return 1
  }
  case "$probe" in
    st-flash) st-flash --reset write "$out/$example.bin" 0x08000000 ;;
    openocd) openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program $out/$example.elf verify reset exit" ;;
    pyocd) pyocd flash -t stm32f411ceux "$out/$example.elf" ;;
    probe-rs) probe-rs download --chip STM32F411CEUx "$out/$example.elf" && probe-rs reset --chip STM32F411CEUx ;;
    dfu)
      require_tool dfu-util "hold BOOT0, reset, then flash over USB DFU" || return 1
      dfu-util -a 0 -d 0483:df11 -s 0x08000000:leave -D "$out/$example.bin"
      ;;
    *)
      echo "probe must be st-flash|openocd|pyocd|probe-rs|dfu" >&2
      return 1
      ;;
  esac
}

# sdk_monitor [port] [baud]
sdk_monitor() {
  python -m serial.tools.miniterm "${1:-/dev/ttyUSB0}" "${2:-115200}"
}

# ---------------------------------------------------------------- new project

# Top-level CMakeLists.txt of a new project named <name>.
stm32_project_cmakelists() {
  local name="$1"
  cat <<EOT
cmake_minimum_required(VERSION 3.20)
set(CMAKE_TOOLCHAIN_FILE \${CMAKE_CURRENT_LIST_DIR}/arm-none-eabi.cmake)
project($name C ASM)
include(\${CMAKE_CURRENT_LIST_DIR}/stm32f411.cmake)
include_directories(\${CMAKE_CURRENT_LIST_DIR})
add_executable($name src/main.c)
target_link_libraries($name stm32f411_hal)
stm32f411_finish($name)
EOT
}

# sdk_new <name> [target]: blinky template into $MCU_ROOT/projects/<name>, pointing at this Cube pack.
sdk_new() {
  local name="${1:-}" target="${2:-blackpill-f411}" dst common="$STM32_EXAMPLES/common"
  [[ -n "$name" ]] || {
    echo "usage: mcu new $SDK <name> [target]" >&2
    return 1
  }
  require_target "$target" || return 1
  dst="$MCU_ROOT/projects/$name"
  [[ ! -e "$dst" ]] || {
    echo "$dst exists" >&2
    return 1
  }
  mkdir -p "$dst"
  cp -r "$STM32_EXAMPLES/blinky-f411/src" "$dst/src"
  cp "$common/"{arm-none-eabi.cmake,stm32f411.cmake,stm32f4xx_hal_conf.h,STM32F411CEUx_FLASH.ld,clock_f411.h} "$dst/"
  sed -i 's|CMAKE_CURRENT_LIST_DIR}/../../STM32CubeF4|MCU_ROOT}/stm32/STM32CubeF4|; s|set(CUBE .*|set(CUBE '"$MCU_ROOT"'/stm32/STM32CubeF4 CACHE PATH "STM32CubeF4")|' "$dst/stm32f411.cmake"
  stm32_project_cmakelists "$name" >"$dst/CMakeLists.txt"
  echo "created $dst (cmake -S $dst -B build)"
}

# ---------------------------------------------------------------- update

# Check out Cube pack tag <tag> of <family> (rollback state recorded first).
stm32_checkout_cube() {
  local family="$1" tag="$2" repo="$SDK_DIR/STM32Cube$1"
  upd_git_rec "STM32Cube$family"
  git -C "$repo" fetch -q --depth 1 origin "refs/tags/$tag:refs/tags/$tag" \
    && git -C "$repo" checkout -q "$tag" \
    && git -C "$repo" submodule update --init --recursive --depth 1
}

# sdk_update [tag]: every Cube pack to its newest vX.Y.Z tag (or the tag given).
sdk_update() {
  UPD_RC=0
  local wanted="${1:-}" family current newest
  for family in "${STM32_FAMILIES[@]}"; do
    current="$(git_ref "$SDK_DIR/STM32Cube$family")"
    newest="$(upd_git_newest_tag "https://github.com/STMicroelectronics/STM32Cube$family" '^v[0-9.]+$')"
    [[ -n "$wanted" && "$wanted" != all ]] && newest="$wanted"
    upd_step "STM32Cube$family" "$current" "$newest" stm32_checkout_cube "$family" "$newest"
  done
  upd_dry_hint
  return "$UPD_RC"
}
