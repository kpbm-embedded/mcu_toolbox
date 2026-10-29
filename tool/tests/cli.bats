#!/usr/bin/env bats
# Argument parsing, help and global flags of the dispatcher.

load helpers

setup() {
  setup_fake_workspace
}

@test "no arguments prints the overview" {
  run "$MCU"
  [ "$status" -eq 0 ]
  [[ "$output" == *"usage: mcu [global flags] <command>"* ]]
}

@test "help lists every command group" {
  run "$MCU" help
  [ "$status" -eq 0 ]
  for group in "build and flash:" "information:" "maintenance:" "environment:" "development:"; do
    [[ "$output" == *"$group"* ]]
  done
}

@test "help <command> and <command> --help work for every command" {
  local cmd
  for cmd in $("$MCU" __complete commands); do
    run "$MCU" help "$cmd"
    [ "$status" -eq 0 ] || {
      echo "help $cmd failed: $output"
      return 1
    }
    [[ "$output" == "usage: mcu $cmd"* ]] || {
      echo "help $cmd: $output"
      return 1
    }
    run "$MCU" "$cmd" --help
    [ "$status" -eq 0 ] || {
      echo "$cmd --help failed: $output"
      return 1
    }
    [[ "$output" == "usage: mcu $cmd"* ]]
  done
}

@test "-h before the command also gives command help" {
  run "$MCU" -h build
  [ "$status" -eq 0 ]
  [[ "$output" == "usage: mcu build"* ]]
}

@test "unknown command is a usage error (exit 2)" {
  run "$MCU" frobnicate
  [ "$status" -eq 2 ]
  [[ "$output" == *"unknown command 'frobnicate'"* ]]
}

@test "help for an unknown command is a usage error" {
  run "$MCU" help frobnicate
  [ "$status" -eq 2 ]
}

@test "unknown sdk fails with the list of sdks" {
  run "$MCU" targets nosuchsdk
  [ "$status" -eq 1 ]
  [[ "$output" == *"unknown sdk 'nosuchsdk'"* ]]
  [[ "$output" == *"stm32"* ]]
}

@test "missing or extra arguments are usage errors" {
  run "$MCU" build stm32
  [ "$status" -eq 2 ]
  run "$MCU" targets
  [ "$status" -eq 2 ]
  run "$MCU" info a b
  [ "$status" -eq 2 ]
}

@test "reserved version-management commands are refused as not implemented (exit 3)" {
  local cmd
  for cmd in install use graph deps rdeps check-deps lock; do
    run "$MCU" "$cmd"
    [ "$status" -eq 3 ]
    [[ "$output" == *"reserved for version management"* ]]
  done
}

@test "where prints the key paths and creates nothing" {
  run "$MCU" where
  [ "$status" -eq 0 ]
  [[ "$output" == *"output"*"$MCU_OUT_BASE"* ]]
  [[ "$output" == *"trash"*"$MCU_TRASH"* ]]
  [ ! -e "$MCU_OUT_BASE" ]
  run "$MCU" where extra
  [ "$status" -eq 2 ]
}

@test "-j N, -jN and --jobs=N set MCU_JOBS for the plugin" {
  run "$WS_MCU" -j 3 build fake board1
  [[ "$output" == *"jobs=3"* ]]
  run "$WS_MCU" build fake board1 -j5
  [[ "$output" == *"jobs=5"* ]]
  run "$WS_MCU" --jobs=7 build fake board1
  [[ "$output" == *"jobs=7"* ]]
  run "$WS_MCU" -j x build fake board1
  [ "$status" -eq 2 ]
}

@test "MCU_JOBS from the environment still works" {
  MCU_JOBS=9 run "$WS_MCU" build fake board1
  [[ "$output" == *"jobs=9"* ]]
}

@test "-n/--dry-run builds nothing" {
  run "$WS_MCU" -n build fake board1
  [ "$status" -eq 0 ]
  [[ "$output" == *"would build fake board1 hello"* ]]
  [ ! -e "$MCU_OUT_BASE/fake/board1/hello/hello.bin" ]
  run "$WS_MCU" build fake board1 --dry-run
  [[ "$output" == *"would build"* ]]
}

@test "-q/--quiet keeps build output in the log only" {
  run "$WS_MCU" -q build fake board1
  [ "$status" -eq 0 ]
  [[ "$output" != *"fake build target"* ]]
  [[ "$output" == *"OK  fake board1 hello"* ]]
  grep -q "fake build target=board1" "$MCU_OUT_BASE/fake/board1/hello/build.log"
}

@test "-v/--verbose shows debug lines" {
  run "$WS_MCU" -v targets fake
  [[ "$output" == *"[debug]"* ]]
  run "$WS_MCU" targets fake
  [[ "$output" != *"[debug]"* ]]
}

@test "--no-color wins over MCU_COLOR=always" {
  MCU_COLOR=always run "$WS_MCU" build fake board1
  [[ "$output" == *$'\033['* ]]
  MCU_COLOR=always run "$WS_MCU" --no-color build fake board1
  [[ "$output" != *$'\033['* ]]
}

@test "combined short flags -yq" {
  "$WS_MCU" build fake board1 >/dev/null
  run "$WS_MCU" -yq clean fake
  [ "$status" -eq 0 ]
  [ ! -e "$MCU_OUT_BASE/fake/board1" ]
}

@test "arguments after -- reach the build tool unchanged" {
  run "$WS_MCU" build fake board1 hello -- -v -DX=1
  [ "$status" -eq 0 ]
  [[ "$output" == *"extra=-v -DX=1"* ]]
}

@test "run passes the command's own flags through" {
  run "$WS_MCU" run fake printenv FAKE_ENV
  [ "$status" -eq 0 ]
  [ "$output" = "1" ]
  run "$WS_MCU" run fake -- bash -c 'echo -n hi'
  [ "$output" = "hi" ]
}

@test "env prints the source line" {
  run "$WS_MCU" env fake
  [ "$output" = "source '$WS/fake/env.sh'" ]
}

@test "setup lists the pins and rejects unknown subcommands" {
  run "$MCU" setup list
  [ "$status" -eq 0 ]
  [[ "$output" == *"arm-gnu-toolchain"*"15.2.rel1"* ]]
  [[ "$output" == *"renode"* ]]
  run "$MCU" setup check no-such-pin
  [ "$status" -eq 2 ]
  run "$MCU" setup frobnicate
  [ "$status" -eq 2 ]
}

@test "every pin has a sha256 or a git commit" {
  local pins
  pins="$(dirname "$REAL_ROOT")/config/pins.tsv"
  run awk -F'\t' '!/^#/ && NF && $5 !~ /^(sha256:[0-9a-f]{64}|git:[0-9a-f]{40})$/ {print; bad=1} END {exit bad}' "$pins"
  [ "$status" -eq 0 ]
}
