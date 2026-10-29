# shellcheck shell=bash disable=SC2034,SC2153
# Fake SDK plugin (contract v2) for the mcu tests: builds write a tiny file, nothing touches real SDKs.
# FAKE_FAIL=1 makes the build fail.

SDK_API=2
SDK_DESC="Fake SDK for the mcu test suite"
SDK_SELFCHECK="board1 hello"
SDK_DEFAULT_EXAMPLE=hello
SDK_FLASH_TOOLS="fakeflash"

sdk_env() {
  # shellcheck source=tests/fixtures/fake/env.sh
  source "$SDK_DIR/env.sh"
}

sdk_doctor() {
  print_ok_missing fake-gcc 0 "fake-gcc 1.0"
}

sdk_versions() {
  print_version fake-sdk 1.0
  print_version fake-tool 2.0
}

sdk_targets() {
  echo "board1  Fake board one (owned)"
  echo "board2  Fake board two"
}

sdk_examples() {
  echo "hello"
  echo "blink"
}

# sdk_build <target> <example> [extra]: writes <example>.bin, echoes what it got.
sdk_build() {
  local target="$1" example="$2" out
  shift 2
  require_target "$target" || return 1
  out="$(build_dir "$target" "$example")"
  mkdir -p "$out"
  echo "fake build target=$target example=$example jobs=$MCU_JOBS env=${FAKE_ENV:-} extra=$*"
  if [[ "${FAKE_FAIL:-0}" == 1 ]]; then
    echo "fake error: compilation failed"
    return 1
  fi
  printf 'firmware' >"$out/$example.bin"
}

sdk_flash() {
  echo "fake flash $*"
}

sdk_cache_dirs() {
  echo "$SDK_DIR/cache"
}

sdk_update() {
  UPD_RC=0
  upd_report fake-sdk 1.0 1.1
  upd_report fake-tool 2.0 2.0
  upd_dry_hint
  return "$UPD_RC"
}
