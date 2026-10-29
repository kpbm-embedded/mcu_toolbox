# source this: Puya PY32 environment (shared Arm GNU toolchain from ../tools + pyocd venv + Puya DFP pack)
if [ -n "${BASH_SOURCE:-}" ]; then _p="${BASH_SOURCE[0]}"; else _p="${(%):-%x}"; fi
_P="$(cd "$(dirname "$_p")" && pwd)"; unset _p
. "$_P/../host/env.sh"
export PY32_DIR="$_P" PY32_TEMPLATE="$_P/py32f0-template"
export PY32_PACK="$PY32_TEMPLATE/Misc/Puya.PY32F0xx_DFP.1.1.7.pack"
export PATH="$_P/../tools/toolchains/arm-gnu-toolchain/bin:$_P/.venv/bin:$PATH"
# pyocd needs the pack explicitly: pyocd flash --pack "$PY32_PACK" -t py32f003x8 app.hex
unset _P
