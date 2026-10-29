# Puya PY32 (mcu/py32)

Boards: PY32F003 / PY32F002A dev boards, PY32F030 / F071 chips. Use `../mcu {doctor|build|flash} py32 ...` or `source env.sh`.

| Component | Version |
|---|---|
| IOsetting/py32f0-template (bundles Puya HAL/LL, CMSIS, linker scripts) | 009bdec (shallow clone) |
| Puya.PY32F0xx_DFP pack | 1.1.7 (Misc/ in template) |
| arm-none-eabi-gcc | 15.2.1 (shared tools/toolchains) |
| pyocd (venv `.venv`) | 0.46.0 |

## Build
`./mcu build py32 py32f003x8` / `py32f002ax5` -> `$MCU_OUT_BASE/py32/<target>/led-toggle/app.{elf,bin,hex}`.
Example is LL/GPIO/LED_Toggle (toggles PA0, UART printf at 115200). Sizes: F003x8 text 5824 B, F002Ax5 text 5824 B.
Local template edit (original kept as `Makefile.orig`): `BUILD_DIR ?= Build` and `APP_DIR ?= User` / `CDIRS := $(APP_DIR)`
so any Examples/ dir builds without copying into User/:
`make MCU_TYPE=PY32F003x8 USE_LL_LIB=y APP_DIR=Examples/PY32F0xx/LL/GPIO/LED_Toggle BUILD_DIR=Build/x`
(do not override CDIRS on the command line: it drops the library dirs the Makefile appends).

## Flash
The pack is not in pyocd's index; pass it explicitly (a project-local `Misc/pyocd.yaml` does this only when run from a dir containing it):
`pyocd flash --pack "$PY32_PACK" -t py32f003x8 app.hex` (env.sh exports PY32_PACK). Check: `pyocd list --targets --pack "$PY32_PACK" | grep -i py32`
(23 targets incl. py32f003x8, py32f002ax5). Probes: DAPLink (WeAct mini), J-Link (`FLASH_PROGRM=jlink`), ST-LINK needs pyocd support.
Via mcu: `./mcu flash py32 py32f003x8 led-toggle [probe-uid]`. Untested on hardware.

## Self-contained
env.sh sources `../host/env.sh` and uses the shared `../tools/toolchains/arm-gnu-toolchain` (no system arm-none-eabi-gcc). `.venv` (pyocd 0.46.0) is rebuilt from managed CPython 3.12 in `host/python`; the old one went to Trash. The Puya pack is in the template.
