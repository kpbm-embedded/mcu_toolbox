# shellcheck shell=bash
# k210 build adapter: out-of-tree CMake project against the SDK (no copy, no SDK edits). Needs `source k210/env.sh`.
tpl_build() { # srcdir target outdir name
  local src="$1" target="$2" out="$3" rt sdk
  # runtime is named in the project's CMakeLists.txt (instantiated templates do not carry their toml)
  if grep -q 'kendryte-freertos-sdk' "$src/CMakeLists.txt"; then rt=freertos; else rt=standalone; fi
  case "$rt" in standalone) sdk="$K210_HOME/kendryte-standalone-sdk";; freertos) sdk="$K210_HOME/kendryte-freertos-sdk";; *) echo "bad runtime $rt"; return 1;; esac
  local b="$out/build"; mkdir -p "$b"; cd "$b" || return 1
  cmake -S "$src" -B "$b" -G "Unix Makefiles" -DSDK="$sdk" -DTOOLCHAIN="$K210_TOOLCHAIN" -DBOARD="$target" \
        -DCMAKE_POLICY_VERSION_MINIMUM=3.5 || return 1
  make -j"$MCU_JOBS" || return 1
  local bin; bin="$(first_file "$b"/*.bin)"; echo "$bin" > "$out/.artifact"
  riscv64-unknown-elf-size "$b"/*.elf 2>/dev/null || true
}
