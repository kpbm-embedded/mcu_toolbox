# shellcheck shell=bash
# bare build adapter: CMake project with CHIP=<target> (bare_chip.cmake/bare.cmake), then the self-test run of the
# firmware in qemu or Renode (run/firmware.sh; chips without an emulator stop after the build), then the project's
# host unit tests (tests/CMakeLists.txt, ctest). check.sh then runs the project's test.sh if it has one.
tpl_build() { # srcdir target outdir name
  local src="$1" t="$2" out="$3" b="$3/build" elf
  cmake -G Ninja -S "$src" -B "$b" -DCHIP="$t" || return 1
  cmake --build "$b" -j"$MCU_JOBS" || return 1
  elf="$b/$4.elf"                                   # the project's own target (named like the project)
  [[ -f $elf ]] || elf="$(find "$b" -maxdepth 1 -name '*.elf' | sort | head -1)"
  [[ -n $elf ]] || { echo "no ELF in $b"; return 1; }
  echo "${elf%.elf}.bin" >"$out/.artifact"
  if [[ "$(cut -d' ' -f1 "${elf%.elf}.run")" != none ]]; then
    "$MCU_BARE/run/firmware.sh" "$elf" || return 1
  fi
  if [[ -f $src/tests/CMakeLists.txt ]]; then
    cmake -G Ninja -S "$src/tests" -B "$out/host-tests" && cmake --build "$out/host-tests" -j"$MCU_JOBS" &&
      ctest --test-dir "$out/host-tests" --output-on-failure || return 1
  fi
  return 0   # test.sh, if the project has one, is run by check.sh after every adapter
}
