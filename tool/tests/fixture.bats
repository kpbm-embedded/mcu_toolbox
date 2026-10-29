#!/usr/bin/env bats
# Build, rebuild, clean, distclean, logs, status, update/outdated and completion, on the fake fixture SDK.

load helpers

setup() {
  setup_fake_workspace
}

@test "build logs to build.log, records the result and prints a summary" {
  run "$WS_MCU" build fake board1
  [ "$status" -eq 0 ]
  [[ "$output" == *"OK  fake board1 hello"* ]]
  [[ "$output" == *"8 B"*"$MCU_OUT_BASE/fake/board1/hello/hello.bin"* ]]
  [[ "$output" == *"log: $MCU_OUT_BASE/fake/board1/hello/build.log"* ]]
  grep -q "fake build target=board1 example=hello" "$MCU_OUT_BASE/fake/board1/hello/build.log"
  grep -q $'\tboard1\thello\tOK\t' "$MCU_OUT_BASE/_state/fake/last-build"
}

@test "the plugin environment is loaded for the build" {
  run "$WS_MCU" build fake board1
  [[ "$output" == *"env=1"* ]]
}

@test "a failing build exits non-zero and says FAIL" {
  FAKE_FAIL=1 run "$WS_MCU" build fake board1 blink
  [ "$status" -ne 0 ]
  [[ "$output" == *"FAIL  fake board1 blink"* ]]
  grep -q $'\tFAIL\t' "$MCU_OUT_BASE/_state/fake/last-build"
}

@test "-q shows the log tail when the build fails" {
  FAKE_FAIL=1 run "$WS_MCU" -q build fake board1
  [ "$status" -ne 0 ]
  [[ "$output" == *"| fake error: compilation failed"* ]]
}

@test "an unknown target fails" {
  run "$WS_MCU" build fake nosuchboard
  [ "$status" -ne 0 ]
  [[ "$output" == *"unknown target 'nosuchboard'"* ]]
}

@test "clean <sdk> <target> moves that build dir to the Trash" {
  "$WS_MCU" build fake board1 >/dev/null
  run "$WS_MCU" clean fake board1
  [ "$status" -eq 0 ]
  [ ! -e "$MCU_OUT_BASE/fake/board1/hello" ]
  [ "$(trash_count)" -eq 1 ]
  ls "$MCU_TRASH/info/"*.trashinfo
  grep -q "Path=$MCU_OUT_BASE/fake/board1/hello" "$MCU_TRASH/info/"*.trashinfo
}

@test "rebuild cleans, then builds" {
  "$WS_MCU" build fake board1 >/dev/null
  touch "$MCU_OUT_BASE/fake/board1/hello/stale.o"
  run "$WS_MCU" rebuild fake board1
  [ "$status" -eq 0 ]
  local moved built
  moved="$(grep -n "to the Trash" <<<"$output" | cut -d: -f1)"
  built="$(grep -n "fake build target" <<<"$output" | cut -d: -f1)"
  [ "$moved" -lt "$built" ]
  [ ! -e "$MCU_OUT_BASE/fake/board1/hello/stale.o" ]
  [ -f "$MCU_OUT_BASE/fake/board1/hello/hello.bin" ]
  [ "$(trash_count)" -eq 1 ]
}

@test "clean <sdk> without --yes refuses when not interactive" {
  "$WS_MCU" build fake board1 >/dev/null
  run "$WS_MCU" clean fake </dev/null
  [ "$status" -ne 0 ]
  [ -d "$MCU_OUT_BASE/fake" ]
  [ "$(trash_count)" -eq 0 ]
}

@test "clean <sdk> --yes moves all builds of the sdk" {
  "$WS_MCU" build fake board1 >/dev/null
  "$WS_MCU" build fake board2 blink >/dev/null
  run "$WS_MCU" clean fake --yes
  [ "$status" -eq 0 ]
  [ ! -e "$MCU_OUT_BASE/fake/board1" ]
  [ ! -e "$MCU_OUT_BASE/fake/board2" ]
  [ "$(trash_count)" -eq 2 ]
}

@test "MCU_YES=1 still confirms" {
  "$WS_MCU" build fake board1 >/dev/null
  MCU_YES=1 run "$WS_MCU" clean all
  [ "$status" -eq 0 ]
  [ ! -e "$MCU_OUT_BASE/fake/board1" ]
}

@test "clean --dry-run lists and moves nothing" {
  "$WS_MCU" build fake board1 >/dev/null
  run "$WS_MCU" -n clean fake --yes
  [ "$status" -eq 0 ]
  [[ "$output" == *"$MCU_OUT_BASE/fake/board1"* ]]
  [ -d "$MCU_OUT_BASE/fake/board1" ]
  [ "$(trash_count)" -eq 0 ]
}

@test "clean --cache trashes the plugin's cache dirs" {
  mkdir -p "$WS/fake/cache"
  touch "$WS/fake/cache/download.tar"
  run "$WS_MCU" clean --cache fake --yes
  [ "$status" -eq 0 ]
  [ ! -e "$WS/fake/cache" ]
  [ "$(trash_count)" -eq 1 ]
}

@test "distclean also takes selfcheck output and state" {
  "$WS_MCU" build fake board1 >/dev/null
  mkdir -p "$MCU_OUT_BASE/_selfcheck/fake"
  echo log >"$MCU_OUT_BASE/_selfcheck/fake.log"
  run "$WS_MCU" distclean fake --yes
  [ "$status" -eq 0 ]
  [ ! -e "$MCU_OUT_BASE/fake/board1" ]
  [ ! -e "$MCU_OUT_BASE/_selfcheck/fake" ]
  [ ! -e "$MCU_OUT_BASE/_selfcheck/fake.log" ]
  [ ! -e "$MCU_OUT_BASE/_state/fake" ]
  [ "$(trash_count)" -eq 4 ]
}

@test "-v build runs check-clean on the SDK" {
  run "$WS_MCU" -v build fake board1
  [ "$status" -eq 0 ]
  [[ "$output" == *"CLEAN fake (0 git checkouts)"* ]]
}

@test "logs shows the newest build log and its tail" {
  "$WS_MCU" build fake board1 >/dev/null
  run "$WS_MCU" logs fake
  [ "$status" -eq 0 ]
  [[ "$output" == "log: $MCU_OUT_BASE/fake/board1/hello/build.log"* ]]
  [[ "$output" == *"fake build target=board1"* ]]
  run "$WS_MCU" logs fake board2
  [ "$status" -ne 0 ]
}

@test "status shows the last build and update check" {
  "$WS_MCU" build fake board1 >/dev/null
  "$WS_MCU" update fake >/dev/null
  run "$WS_MCU" status
  [ "$status" -eq 0 ]
  [[ "$output" == *"SDK"*"LAST BUILD"*"UPDATES"* ]]
  [[ "$output" == *"fake"*"fake-sdk 1.0"*"OK board1/hello"*"yes (1)"* ]]
}

@test "update is a dry run and prints the summary table" {
  run "$WS_MCU" update fake
  [ "$status" -eq 0 ]
  [[ "$output" == *"=== update summary"* ]]
  [[ "$output" == *"fake"*"fake-sdk"*"1.0 -> 1.1"*"available (dry run)"* ]]
  [[ "$output" == *"fake"*"fake-tool"*"2.0"*"current"* ]]
}

@test "outdated prints only components that are not current" {
  run "$WS_MCU" outdated
  [ "$status" -eq 0 ]
  [[ "$output" == *"fake-sdk"*"1.0 -> 1.1"* ]]
  [[ "$output" != *"fake-tool"* ]]
  [[ "$output" == *"1 components current"* ]]
}

@test "flash gets target, default example and port" {
  run "$WS_MCU" flash fake board1
  [ "$output" = "fake flash board1 hello" ]
  run "$WS_MCU" flash fake board1 blink /dev/ttyX
  [ "$output" = "fake flash board1 blink /dev/ttyX" ]
}

@test "completion: commands, sdks, targets and examples" {
  run "$WS_MCU" __complete next
  [[ "$output" == *"rebuild"* ]]
  run "$WS_MCU" __complete next build
  [ "$output" = "fake" ]
  run "$WS_MCU" __complete next build fake
  [ "$output" = $'board1\nboard2' ]
  run "$WS_MCU" __complete next -q build fake board1
  [ "$output" = $'hello\nblink' ]
  run "$WS_MCU" __complete next clean --cache
  [ "$output" = $'fake\nall' ]
}

@test "bash completion script completes through mcu" {
  source "$REAL_ROOT/completion/mcu.bash"
  COMP_WORDS=("$WS_MCU" build fake b)
  COMP_CWORD=3
  _mcu_complete
  [ "${COMPREPLY[*]}" = "board1 board2" ]
  COMP_WORDS=("$WS_MCU" reb)
  COMP_CWORD=1
  _mcu_complete
  [ "${COMPREPLY[*]}" = "rebuild" ]
  COMP_WORDS=("$WS_MCU" --no)
  _mcu_complete
  [ "${COMPREPLY[*]}" = "--no-color" ]
}

@test "zsh completion script is valid zsh" {
  command -v zsh >/dev/null || skip "no zsh"
  zsh -n "$REAL_ROOT/completion/_mcu"
}

# new: projects/<name> from templates/<sdk>/*minimal, picked by target (instantiate.py "auto").
setup_fake_templates() {
  mkdir -p "$WS/templates/fake/minimal" "$WS/templates/fake/other-minimal" "$WS/projects"
  ln -s "$REAL_ROOT/templates/tools" "$WS/templates/tools"
  printf 'name = "minimal"\nsdk = "fake"\ndescription = "d"\ntargets = ["board1"]\ndefault_target = "board1"\n' >"$WS/templates/fake/minimal/template.toml"
  printf 'name = "other-minimal"\nsdk = "fake"\ndescription = "d"\ntargets = ["board2"]\ndefault_target = "board2"\n' >"$WS/templates/fake/other-minimal/template.toml"
  printf 'app {{NAME}} for {{TARGET}} root {{MCU_ROOT}}\n' >"$WS/templates/fake/minimal/main.c"
  printf 'other {{NAME}} {{TARGET}}\n' >"$WS/templates/fake/other-minimal/main.c"
  mkdir -p "$WS/templates/fake/minimal-b3" "$WS/templates/fake/rtos"
  printf 'name = "minimal-b3"\nsdk = "fake"\ndescription = "d"\ntargets = ["board3"]\ndefault_target = "board3"\n' >"$WS/templates/fake/minimal-b3/template.toml"
  printf 'b3 {{NAME}}\n' >"$WS/templates/fake/minimal-b3/main.c"
  printf 'name = "rtos"\nsdk = "fake"\ndescription = "d"\ntargets = ["board4"]\ndefault_target = "board4"\n' >"$WS/templates/fake/rtos/template.toml"
}

@test "new instantiates the minimal template into projects/<name>" {
  setup_fake_templates
  run "$WS_MCU" new fake p1
  [ "$status" -eq 0 ]
  [ "$(cat "$WS/projects/p1/main.c")" = "app p1 for board1 root $WS" ]
  [ ! -e "$WS/projects/p1/template.toml" ]
}

@test "new picks the *minimal template whose targets include the target" {
  setup_fake_templates
  run "$WS_MCU" new fake p2 board2
  [ "$status" -eq 0 ]
  [ "$(cat "$WS/projects/p2/main.c")" = "other p2 board2" ]
}

@test "new also picks minimal-<suffix> templates, never non-minimal ones" {
  setup_fake_templates
  run "$WS_MCU" new fake p6 board3
  [ "$status" -eq 0 ]
  [ "$(cat "$WS/projects/p6/main.c")" = "b3 p6" ]
  run "$WS_MCU" new fake p7 board4
  [ "$status" -ne 0 ]
  [ ! -e "$WS/projects/p7" ]
}

@test "new refuses an existing project, an unknown target and a bad name" {
  setup_fake_templates
  mkdir -p "$WS/projects/p3"
  run "$WS_MCU" new fake p3
  [ "$status" -ne 0 ]
  run "$WS_MCU" new fake p4 nosuchboard
  [ "$status" -ne 0 ]
  [ ! -e "$WS/projects/p4" ]
  run "$WS_MCU" new fake ../evil
  [ "$status" -ne 0 ]
}

@test "new without templates falls back to the plugin (unsupported here)" {
  run "$WS_MCU" new fake p5
  [ "$status" -ne 0 ]
  [[ "$output" == *"sdk_new"* ]]
}
