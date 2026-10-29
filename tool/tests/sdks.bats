#!/usr/bin/env bats
# Read-only commands against every SDK of the real workspace, and plugin validation.

load helpers

setup() {
  setup_isolation
}

@test "list shows every sdk with a description" {
  run "$MCU" list
  [ "$status" -eq 0 ]
  local sdk
  for sdk in $("$MCU" __complete sdks); do
    grep -qE "^$sdk +[^ ]" <<<"$output" || {
      echo "missing $sdk"
      return 1
    }
  done
}

@test "info works for every sdk" {
  local sdk
  for sdk in $("$MCU" __complete sdks); do
    run "$MCU" info "$sdk"
    [ "$status" -eq 0 ] || {
      echo "info $sdk: $output"
      return 1
    }
    [[ "$output" == "$sdk - "* ]]
    [[ "$output" == *"directory"*"$sdk"* ]]
  done
}

@test "targets and examples work for every buildable sdk" {
  local sdk
  for sdk in $("$MCU" __complete sdks); do
    # here-strings, not pipes: under bats' pipefail, grep -q closing the pipe early can fail the producer
    grep -q 'commands .*build' <<<"$("$MCU" info "$sdk")" || continue
    run "$MCU" targets "$sdk"
    [ "$status" -eq 0 ] || {
      echo "targets $sdk: $output"
      return 1
    }
    [ -n "$output" ]
    run "$MCU" examples "$sdk"
    [ -n "$output" ]
    # v1 plugins may end sdk_examples with a false test (k210, phase-2 fix); v2 plugins must exit 0
    if grep -q 'contract v2' <<<"$("$MCU" check-plugins "$sdk")"; then
      [ "$status" -eq 0 ] || {
        echo "examples $sdk: $output"
        return 1
      }
    fi
  done
}

@test "targets of an sdk without targets is reported as unsupported" {
  run "$MCU" targets host
  [ "$status" -eq 1 ]
  [[ "$output" == *"not supported"* ]]
}

@test "check-plugins passes for every plugin" {
  run "$MCU" check-plugins
  [ "$status" -eq 0 ]
  [[ "$output" != *"FAIL"* ]]
}

@test "migrated plugins are contract v2" {
  run "$MCU" check-plugins stm32
  [[ "$output" == *"OK   stm32     contract v2"* ]]
  run "$MCU" check-plugins wch
  [[ "$output" == *"OK   wch       contract v2"* ]]
}

@test "check-plugins catches rm and absolute OS tool paths" {
  setup_fake_workspace
  local remove="rm" force="-rf"
  # shellcheck disable=SC2016  # $MCU_OUT is plugin code written literally
  printf 'sdk_clean() { %s %s "$MCU_OUT"; }\nsdk_monitor() { /usr/bin/screen /dev/ttyUSB0; }\n' "$remove" "$force" >>"$WS/fake/sdk.sh"
  run "$WS_MCU" check-plugins fake
  [ "$status" -eq 1 ]
  [[ "$output" == *"FAIL fake      calls rm"* ]]
  [[ "$output" == *"FAIL fake      calls OS tools by path"* ]]
}

@test "check-plugins catches a missing required function" {
  setup_fake_workspace
  sed -i 's/^sdk_versions()/fake_versions()/' "$WS/fake/sdk.sh"
  run "$WS_MCU" check-plugins fake
  [ "$status" -eq 1 ]
  [[ "$output" == *"required function sdk_versions is missing"* ]]
}

@test "check-clean: builds left the migrated SDKs' checkouts unchanged" {
  run "$MCU" check-clean stm32
  [ "$status" -eq 0 ] || {
    echo "$output"
    return 1
  }
  run "$MCU" check-clean wch
  [ "$status" -eq 0 ] || {
    echo "$output"
    return 1
  }
}

@test "bulk clean never selects SDK sources linked into the build disk" {
  setup_fake_workspace
  mkdir -p "$MCU_OUT_BASE/fake/board1" "$MCU_OUT_BASE/fake/board2" "$MCU_OUT_BASE/fake/dl"
  ln -s "$MCU_OUT_BASE/fake/board2" "$WS/fake/linked-src"
  run "$WS_MCU" clean fake --yes
  [ "$status" -eq 0 ]
  [ ! -e "$MCU_OUT_BASE/fake/board1" ]
  [ -d "$MCU_OUT_BASE/fake/board2" ]
  [ -d "$MCU_OUT_BASE/fake/dl" ]
}
