# shellcheck shell=bash
# stm32 build adapter: out-of-tree CMake app against the shared board layer (mcu/stm32/boards). Needs `source stm32/env.sh` (done by check.sh).
# The target is the board name (blackpill-f411 | weact-g431 | mini-h743 | bluepill-f103), passed as -DBOARD.
tpl_build() { # srcdir target outdir name
  local src="$1" t="$2" out="$3" b="$3/build"
  cmake -G Ninja -S "$src" -B "$b" -DBOARD="$t" || return 1
  cmake --build "$b" -j"$MCU_JOBS" || return 1
  local f; f="$(find "$b" -maxdepth 1 -name '*.bin' | head -1)"; [[ -n $f ]] && echo "$f" > "$out/.artifact"
  # templates with host unit tests: build + run them too (host compiler, Unity)
  if [[ -f $src/tests/CMakeLists.txt ]]; then
    cmake -G Ninja -S "$src/tests" -B "$out/host-tests" && cmake --build "$out/host-tests" -j"$MCU_JOBS" &&
      ctest --test-dir "$out/host-tests" --output-on-failure || return 1
  fi
}
