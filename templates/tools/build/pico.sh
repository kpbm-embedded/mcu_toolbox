# shellcheck shell=bash
# pico build adapter: out-of-tree CMake app (pico_sdk_import.cmake in the project). Needs `source pico/env.sh` (done by check.sh).
tpl_build() { # srcdir target outdir name
  local src="$1" t="$2" out="$3" bp tc="$PICO_TOOLCHAIN_PATH"
  case "$t" in
    pico) bp=(-DPICO_BOARD=pico);; zero) bp=(-DPICO_BOARD=waveshare_rp2040_zero);; pico2) bp=(-DPICO_BOARD=pico2);;
    pico2-riscv) bp=(-DPICO_BOARD=pico2 -DPICO_PLATFORM=rp2350-riscv); tc="$PICO_TOOLCHAIN_PATH_RISCV";;
    rp2350-zero) bp=(-DPICO_BOARD=waveshare_rp2350_zero);;
    rp2350-zero-riscv) bp=(-DPICO_BOARD=waveshare_rp2350_zero -DPICO_PLATFORM=rp2350-riscv); tc="$PICO_TOOLCHAIN_PATH_RISCV";;
    *) echo "unknown pico target $t"; return 1;; esac
  local b="$out/build"
  # shellcheck disable=SC2154  # picotool_DIR is exported by pico/env.sh (sourced by check.sh)
  cmake -G Ninja -S "$src" -B "$b" "${bp[@]}" -DPICO_TOOLCHAIN_PATH="$tc" -Dpicotool_DIR="$picotool_DIR" || return 1
  cmake --build "$b" -j"$MCU_JOBS" || return 1
  local u; u="$(find "$b" -maxdepth 1 -name '*.uf2' | head -1)"; [[ -n $u ]] && echo "$u" > "$out/.artifact"
  # templates with host unit tests: build + run them too
  if [[ -f $src/tests/CMakeLists.txt ]]; then
    cmake -G Ninja -S "$src/tests" -B "$out/host-tests" && cmake --build "$out/host-tests" -j"$MCU_JOBS" &&
      ctest --test-dir "$out/host-tests" --output-on-failure || return 1
  fi
}
