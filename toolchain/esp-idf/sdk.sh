# esp-idf plugin (see ../PLUGIN.md)
SDK_SELFCHECK="esp32c3 get-started/blink"
SDK_DESC="ESP-IDF (ESP32, S3, C3, C6, P4) with local tools dir + uv python 3.12"
sdk_env() { source "$SDK_DIR/env.sh"; }
sdk_doctor() {
  local rc=0 t
  for t in idf.py cmake ninja python xtensa-esp32s3-elf-gcc riscv32-esp-elf-gcc; do
    if command -v $t >/dev/null; then echo "ok $t $($t --version 2>&1 | head -n1)"; else echo "MISSING $t"; rc=1; fi; done
  echo "ok IDF_PATH=$IDF_PATH IDF_TOOLS_PATH=$IDF_TOOLS_PATH"
  return $rc; }
sdk_versions() {
  echo "esp-idf   $(git -C "$IDF_PATH" describe --tags 2>/dev/null) ($(idf.py --version 2>&1 | tail -n1))"
  echo "python    $(python --version 2>&1)"
  echo "xtensa    $(xtensa-esp32s3-elf-gcc -dumpfullversion)"
  echo "riscv     $(riscv32-esp-elf-gcc -dumpfullversion)"; }
sdk_targets() { cat <<'T'
esp32     ESP32 WROOM-32 CH340C
esp32s3   ESP32-S3 devkit N16R8 / S3-Zero / HMI display
esp32c3   ESP32-C3 SuperMini / Zero
esp32c6   ESP32-C6 (WT9932P4-TINY companion)
esp32p4   ESP32-P4 (WT9932P4-TINY)
T
}
sdk_examples() { echo "get-started/blink (default)"; echo "get-started/hello_world"; (cd "$IDF_PATH/examples" && ls -d */*/ | sed 's#/$##' | head -n 200); }
sdk_build() {
  local t=${1:?target} ex=${2:-get-started/blink}
  shift $(( $# >= 2 ? 2 : $# )); [ "${1:-}" = "--" ] && shift
  local src="$IDF_PATH/examples/$ex" d="$MCU_OUT/$t/$(echo "$ex" | tr / _)"
  [ -d "$src" ] || src="$ex"; [ -f "$src/CMakeLists.txt" ] || { echo "no such example: $ex"; return 1; }
  # sdkconfig is generated inside the build dir (idf.py -B + SDKCONFIG) so the SDK tree stays clean
  idf.py -C "$src" -B "$d" -DSDKCONFIG="$d/sdkconfig" -DIDF_TARGET=$t "$@" build || return 1
  ls -l "$d"/*.bin "$d"/*.elf 2>/dev/null | grep -v bootloader.elf; }
sdk_flash() {
  local t=${1:?target} ex=${2:-get-started/blink} p=${3:-}; local d="$MCU_OUT/$t/$(echo "$ex" | tr / _)"
  idf.py -C "$IDF_PATH/examples/$ex" -B "$d" ${p:+-p $p} flash; }
sdk_monitor() { idf.py ${1:+-p $1} ${2:+-b $2} monitor; }
sdk_clean() { local d="$MCU_OUT/${1:?target}/$(echo "${2:-get-started/blink}" | tr / _)"; [ -d "$d" ] && mv "$d" "$HOME/.local/share/Trash/files/esp-$1-$(date +%s)" && echo "moved $d to Trash"; }
_e_do() { local new="$1"; upd_git_rec esp-idf
  git -C "$IDF_PATH" fetch -q --depth 1 origin tag "$new" && git -C "$IDF_PATH" checkout -q "$new" && git -C "$IDF_PATH" submodule update --init --recursive --depth 1 -j8 || return 1
  "$SDK_DIR/install.sh" >/dev/null; }
sdk_update() {
  UPD_RC=0
  local cur new; cur=$(git -C "$IDF_PATH" describe --tags)
  new=${1:-$(upd_git_newest_tag https://github.com/espressif/esp-idf '^v[0-9]+\.[0-9]+(\.[0-9]+)?$')}
  upd_step esp-idf "$cur" "$new" _e_do "$new"
  upd_dry_hint
  return "$UPD_RC"
}
