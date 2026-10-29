# source me (bash/zsh): K210 toolchain + kflash venv
if [ -n "${BASH_SOURCE:-}" ]; then _k="${BASH_SOURCE[0]}"; else _k="${(%):-%x}"; fi
K210_HOME=$(cd "$(dirname "$_k")" && pwd); unset _k
. "$K210_HOME/../host/env.sh"
export K210_HOME
export K210_BUILD="${MCU_OUT_BASE:-$K210_HOME/../../build}/k210"   # mcu sets MCU_OUT_BASE
export K210_TOOLCHAIN=$K210_HOME/toolchain/kendryte-toolchain/bin
export PATH=$K210_TOOLCHAIN:$K210_HOME/.venv/bin:$PATH
