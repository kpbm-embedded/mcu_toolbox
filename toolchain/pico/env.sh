# source this: self-contained Pico SDK 2.3.1 env (no system toolchain/cmake/python)
_R="$(cd "$(dirname "${BASH_SOURCE[0]:-$0}")" && pwd)"
_RH="$(cd "$(dirname "${BASH_SOURCE[0]:-$0}")/.." && pwd)"; . "$_RH/host/env.sh"
export PICO_SDK_PATH="$_R/pico-sdk"
export PICO_EXAMPLES_PATH="$_R/pico-examples"
export picotool_DIR="$_R/picotool"
export PICO_TOOLCHAIN_PATH="$_R/../tools/toolchains/arm-gnu-toolchain"
export PICO_TOOLCHAIN_PATH_RISCV="$_R/toolchain/riscv"
export PATH="$_R/bin:$PICO_TOOLCHAIN_PATH/bin:$_R/toolchain/riscv/bin:$PATH"
export PICO_BUILD_ROOT="${MCU_OUT_BASE:-$_R/../../build}/pico"   # mcu sets MCU_OUT_BASE
