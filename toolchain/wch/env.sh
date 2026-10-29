# shellcheck shell=bash
# wch environment: source it from bash or zsh (xPack riscv-none-elf-gcc, minichlink, wlink, wchisp).
# In bash BASH_SOURCE names this file; in a sourced zsh file $0 does.
if [ -n "${BASH_SOURCE:-}" ]; then _mcu_env_file="${BASH_SOURCE[0]}"; else _mcu_env_file="$0"; fi
_mcu_wch="$(cd "$(dirname "$_mcu_env_file")" && pwd)"
# shellcheck source=SCRIPTDIR/../host/env.sh
. "$_mcu_wch/../host/env.sh"

# Component locations (one place): stable links flipped by `mcu update`, never versioned paths
export WCH_DIR="$_mcu_wch"
export CH32FUN_REPO="$_mcu_wch/ch32fun"
export PATH="$_mcu_wch/toolchain/xpack-riscv-none-elf-gcc/bin:$_mcu_wch/bin:$PATH"
unset _mcu_env_file _mcu_wch
