_R="$(cd "$(dirname "${BASH_SOURCE[0]:-$0}")" && pwd)"
. "$_R/../host/env.sh"
export BL_SDK_BASE="$_R/bouffalo_sdk"
export PATH="$_R/toolchain/xuantie-gcc-riscv-v2.10.2-linux-amd64/bin:$_R/venv/bin:$PATH"
