# shellcheck shell=bash
SDK_DESC="Puya PY32F0xx: IOsetting/py32f0-template (HAL/LL) + arm-none-eabi-gcc + pyocd"
SDK_SELFCHECK="py32f003x8 led-toggle"
sdk_env() { source "$SDK_DIR/env.sh"; }
sdk_doctor() {
  local rc=0
  command -v arm-none-eabi-gcc >/dev/null && echo "ok      arm-gcc $(arm-none-eabi-gcc -dumpversion) newlib=$([[ -f $(arm-none-eabi-gcc -print-file-name=libc.a) ]] && echo yes || echo NO)" || { echo "MISSING arm-none-eabi-gcc"; rc=1; }
  command -v make >/dev/null && echo "ok      make" || { echo "MISSING make"; rc=1; }
  command -v pyocd >/dev/null && echo "ok      pyocd $(pyocd --version)" || { echo "MISSING pyocd (.venv)"; rc=1; }
  [[ -f $PY32_PACK ]] && echo "ok      Puya DFP pack $(basename "$PY32_PACK")" || { echo "MISSING pack"; rc=1; }
  [[ -f $PY32_TEMPLATE/Makefile ]] && echo "ok      py32f0-template" || { echo "MISSING template"; rc=1; }
  return $rc
}
sdk_versions() {
  echo "py32f0-template $(git -C "$PY32_TEMPLATE" rev-parse HEAD)"
  echo "arm-none-eabi-gcc $(arm-none-eabi-gcc -dumpversion) (shared tools/toolchains)"
  echo "pyocd $(pyocd --version)"
  echo "pack $(basename "$PY32_PACK")"
}
sdk_targets() { printf '%s\n' "py32f003x8  PY32F003 dev board (64K/8K)" "py32f002ax5  PY32F002A dev board (20K/3K)" "py32f030x8  PY32F030 chips" "py32f071x8  PY32F071 chips (HAL only)"; }
sdk_examples() {
  echo "led-toggle  LL/GPIO/LED_Toggle (default; PA0 toggle + UART printf)"
  echo "<path>      any dir under py32f0-template/Examples/ e.g. PY32F0xx/LL/GPIO/EXTI_Toggle (LL) or HAL/... "
}
_ex() { case "$1" in led-toggle) echo PY32F0xx/LL/GPIO/LED_Toggle;; *) echo "$1";; esac; }
_mcuty() { case "$1" in py32f003x8) echo PY32F003x8;; py32f002ax5) echo PY32F002Ax5;; py32f030x8) echo PY32F030x8;; py32f071x8) echo PY32F071x8;; *) echo "unknown target $1" >&2; return 1;; esac; }
sdk_build() {
  local t="${1:?target}" e="${2:-led-toggle}" m ex ll=y; m="$(_mcuty "$t")" || return 1; ex="$(_ex "$e")"
  [[ $ex == */HAL/* ]] && ll=n
  local out="$MCU_OUT/$t/$e"; mkdir -p "$out"
  ( cd "$PY32_TEMPLATE" && make -j"$MCU_JOBS" MCU_TYPE="$m" USE_LL_LIB=$ll APP_DIR="Examples/$ex" BUILD_DIR="Build/$t-$e" ) | tail -4
  cp "$PY32_TEMPLATE/Build/$t-$e"/app.{elf,bin,hex} "$out"/
  ls -l "$out"; arm-none-eabi-size "$out/app.elf"
}
sdk_flash() { # target [example] [probe-uid]
  local t="${1:?target}" e="${2:-led-toggle}" uid="${3:-}"
  local f="$MCU_OUT/$t/$e/app.hex"; [[ -f $f ]] || { echo "build first" >&2; return 1; }
  pyocd flash --pack "$PY32_PACK" -t "$t" ${uid:+-u "$uid"} "$f"
}
sdk_monitor() { python3 -m serial.tools.miniterm "${1:-/dev/ttyUSB0}" "${2:-115200}"; }
sdk_clean() {
  local d="$MCU_OUT/${1:?target}/${2:-led-toggle}"
  [[ -d $d ]] || { echo "nothing at $d"; return 0; }
  mkdir -p ~/.local/share/Trash/files; mv "$d" ~/.local/share/Trash/files/"py32-$1-${2:-led-toggle}-$(date +%s)"; echo "moved to Trash"
}
_y_do_tpl() { upd_git_rec "${PY32_TEMPLATE#"$SDK_DIR"/}"
  # the template Makefile carries a local edit (APP_DIR, BUILD_DIR ?=); keep it via stash
  git -C "$PY32_TEMPLATE" stash -q; git -C "$PY32_TEMPLATE" pull -q --ff-only; local rc=$?; git -C "$PY32_TEMPLATE" stash pop -q || true; return $rc; }
_y_do_pyocd() { upd_rec pip .venv/bin/python "pyocd==$1"; uv pip install -q --python "$SDK_DIR/.venv/bin/python" -U pyocd; }
sdk_update() {
  UPD_RC=0
  local cur up; cur="$(git -C "$PY32_TEMPLATE" rev-parse --short=12 HEAD)"; up="$(git -C "$PY32_TEMPLATE" ls-remote origin HEAD | cut -c1-12)"
  upd_step py32f0-template "$cur" "${up:-$cur}" _y_do_tpl
  local c; c="$(pyocd --version | head -1)"
  upd_step pyocd "$c" "$(upd_pypi_latest pyocd)" _y_do_pyocd "$c"
  upd_dry_hint
  return "$UPD_RC"
}
