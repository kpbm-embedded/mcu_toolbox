# shellcheck shell=bash
# bare environment (bash or zsh): host layer, Arm GNU and xPack RISC-V compilers, qemu (Arm, RISC-V), Renode.
if [ -n "${BASH_SOURCE:-}" ]; then _b_env="${BASH_SOURCE[0]}"; else _b_env="$0"; fi
_b_dir="$(cd -P "$(dirname "$_b_env")" && pwd -P)"
# shellcheck source=/dev/null  # the host layer, resolved at run time
. "$_b_dir/../host/env.sh"
# Component locations (stable links only; mcu setup installs the versions behind them)
export MCU_BARE="$_b_dir"
export PATH="$_b_dir/../tools/toolchains/arm-gnu-toolchain/bin:$_b_dir/../wch/toolchain/xpack-riscv-none-elf-gcc/bin:$_b_dir/qemu-arm/bin:$_b_dir/qemu-riscv/bin:$_b_dir/renode:$PATH"
unset _b_env _b_dir
