# shellcheck shell=bash disable=SC2016
# (SC2016: help text shows $VARS and `commands` literally on purpose)
# help.sh - `mcu help` and `mcu help <command>` (also `mcu <command> --help`).
# The command list itself (name, group, usage, summary) is the table in ../mcu; details live here.

# Overview: usage, global flags and every command grouped.
help_overview() {
  cat <<'EOF'
mcu - one entry point for every MCU SDK/toolchain in this workspace (self-contained, no OS packages).

usage: mcu [global flags] <command> [args...]
       mcu help <command>        details and examples for one command

global flags (anywhere before a '--'):
  -y, --yes        apply / confirm without asking (same as MCU_YES=1)
  -n, --dry-run    show what would change, change nothing (MCU_DRY_RUN=1)
  -v, --verbose    debug messages (MCU_VERBOSE=1)
  -q, --quiet      only results; build output goes to the log only (MCU_QUIET=1)
  -j N, --jobs N   parallel jobs (MCU_JOBS, default 16)
  --no-color       plain output (also NO_COLOR=1 or MCU_COLOR=never)
  -h, --help       help for the command
EOF
  local group name usage summary current=""
  while IFS=';' read -r name group usage summary; do
    [[ "$group" == hidden || "$group" == reserved ]] && continue
    if [[ "$group" != "$current" ]]; then
      printf '\n%s:\n' "$group"
      current="$group"
    fi
    if ((${#usage} > 40)); then
      printf '  mcu %s\n  %-44s %s\n' "$usage" "" "$summary"
    else
      printf '  %-44s %s\n' "mcu $usage" "$summary"
    fi
  done < <(command_table)
  printf '\nreserved for version management (not implemented yet, exit 3): %s\n' "$(command_names_in reserved | tr '\n' ' ')"
  cat <<'EOF'

environment variables:
  MCU_OUT_BASE     build output base (default $BOX_OUT/<this ID path>/build)
  MCU_TRASH        Trash directory for clean/prune/distclean (default: the volume's Trash)
  MCU_YES MCU_DRY_RUN MCU_VERBOSE MCU_QUIET MCU_JOBS MCU_COLOR   same as the global flags
  NO_COLOR         plain output

exit status: 0 ok, 1 failure, 2 usage error, 3 not implemented

examples:
  mcu list
  mcu build stm32 blackpill-f411
  mcu -n clean all
  mcu help build
EOF
  printf 'docs: %s/README.md (user guide), %s/PLUGIN.md (plugin contract)\n' "$MCU_ROOT" "$MCU_ROOT"
}

# Help for one command: usage line, summary and the details below.
help_command() {
  local cmd="$1" name group usage summary
  while IFS=';' read -r name group usage summary; do
    [[ "$name" == "$cmd" ]] || continue
    printf 'usage: mcu %s\n\n%s\n' "$usage" "$summary"
    help_details "$cmd"
    return 0
  done < <(command_table)
  die_usage help "unknown command '$cmd'"
}

# Extra text and examples per command.
help_details() {
  case "$1" in
    list) printf '\nexample:\n  mcu list\n' ;;
    info) printf '\nShows description, directories, README/VERSIONS paths, selfcheck spec, flash tools,\nsupported commands, targets and examples.\n\nexample:\n  mcu info wch\n' ;;
    status) printf '\nTable per SDK: first version line, last build, last selfcheck, update availability\n(from the last `mcu outdated`/`mcu update`), disk use of the SDK dir and its build output.\nDisk use is cached for 24h; --refresh measures again.\n\nexamples:\n  mcu status\n  mcu status stm32 --refresh\n' ;;
    doctor) printf '\nPrints ok/MISSING per required tool; non-zero exit when something is missing.\n\nexamples:\n  mcu doctor wch\n  mcu doctor          # all SDKs\n' ;;
    versions) printf '\nexample:\n  mcu versions zephyr\n' ;;
    targets) printf '\nexample:\n  mcu targets stm32\n' ;;
    examples) printf '\nexample:\n  mcu examples wch\n' ;;
    where) printf '\nRead-only; one "name  path" line each. The Trash line is MCU_TRASH or "per volume".\n\nexample:\n  mcu where\n' ;;
    build) printf '\nBuilds into $MCU_OUT_BASE/<sdk>/<target>/<example> (default %s), logs to build.log there\nand ends with OK/FAIL, duration and the artifacts with sizes. Arguments after -- go to the build tool.\n\nexamples:\n  mcu build stm32 blackpill-f411\n  mcu build wch ch32v003 blink\n  mcu -q build zephyr rpi_pico -- -DCONFIG_DEBUG=y\n' "${MCU_OUT_BASE:-}" ;;
    rebuild) printf '\nSame as `mcu clean <sdk> <target> [example]` followed by `mcu build ...`.\n\nexample:\n  mcu rebuild wch ch32v003 blink\n' ;;
    flash) printf '\nFlashes the last build of that target/example with the tool for the board\n(see `mcu info <sdk>` for the flash tools).\n\nexamples:\n  mcu flash stm32 blackpill-f411\n  mcu flash wch ch32v003 blink wlink\n' ;;
    monitor) printf '\nexample:\n  mcu monitor stm32 /dev/ttyUSB0 115200\n' ;;
    clean) printf '\nEverything goes to the Trash (never deleted). Forms:\n  clean <sdk> <target> [example]   one build dir (no question asked)\n  clean <sdk>                      all build outputs of the SDK\n  clean all                        all build outputs of every SDK\n  clean --cache <sdk|all>          SDK caches (download caches...), when the plugin defines them\nThe bulk forms ask first unless --yes; --dry-run only lists.\n\nexamples:\n  mcu clean stm32 blackpill-f411\n  mcu clean stm32 --yes\n  mcu clean --cache wch -n\n' ;;
    distclean) printf '\nAll build outputs, the selfcheck output/log and the state records (mcu status) of the SDK.\nSources, toolchains, venvs and downloads stay. Asks first unless --yes.\n\nexample:\n  mcu distclean wch --yes\n' ;;
    logs) printf '\nShows the newest build log of the scope (or the selfcheck log when that is newer) and its tail.\n\nexamples:\n  mcu logs stm32\n  mcu logs wch ch32v003 blink --lines 100\n' ;;
    update) printf '\nDry run by default: every component, installed -> latest. With --yes it applies, then runs the\npost-checks (patch-rpath, selfcheck, libcheck); any failure rolls back automatically.\n`update all` continues past a failing SDK and ends with a summary table.\n\nexamples:\n  mcu update wch\n  mcu update wch --yes\n  mcu update all\n' ;;
    outdated) printf '\nDry-run update of every SDK; prints only components that are not current.\nThe result is cached for `mcu status`.\n\nexample:\n  mcu outdated\n' ;;
    rollback) printf '\nReplays <sdk>/.update-state (written by the last applied update) in reverse.\n\nexample:\n  mcu rollback wch\n' ;;
    prune) printf '\nLists versioned dirs older than the previous version; --yes moves them to the Trash.\n\nexample:\n  mcu prune host --yes\n' ;;
    selfcheck) printf '\nBuilds each SDK_SELFCHECK target with PATH limited to host/sysbin (OS base utilities only).\nLog: $MCU_OUT_BASE/_selfcheck/<sdk>.log.\n\nexamples:\n  mcu selfcheck stm32\n  mcu selfcheck\n' ;;
    libcheck) printf '\nexample:\n  mcu libcheck wch\n' ;;
    check-plugins) printf '\nValidates every plugin against PLUGIN.md: required functions and variables, no rm, no /usr/bin.\nv1 (compat) plugins get warnings for v2-only rules.\n\nexample:\n  mcu check-plugins\n' ;;
    shell) printf '\nexample:\n  mcu shell wch\n' ;;
    run) printf '\nexample:\n  mcu run stm32 -- arm-none-eabi-gcc --version\n' ;;
    env) printf '\nexample:\n  eval "$(mcu env zephyr)"\n' ;;
    new) printf '\nexample:\n  mcu new stm32 myproj blackpill-f411\n' ;;
    lint) printf '\nshellcheck over the framework and migrated (v2) plugins; v1 plugin findings are listed as\npending. --all makes them count too.\n\nexample:\n  mcu lint\n' ;;
    fmt) printf '\nshfmt -i 2 -ci -bn over the framework and v2 plugins (files are replaced atomically).\n--check only lists files that would change.\n\nexamples:\n  mcu fmt --check\n  mcu fmt\n' ;;
    test) printf '\nRuns tests/*.bats with the vendored bats-core. Extra args go to bats.\n\nexample:\n  mcu test\n' ;;
    devtools) printf '\nInstalls the pinned, sha256-verified shellcheck, shfmt (host/devtools) and bats-core (tests/bats).\n\nexamples:\n  mcu devtools          # status\n  mcu devtools install\n' ;;
    check-clean) printf '\nFor every git checkout inside the SDK dirs (submodules, west/repo projects included) lists tracked\nfiles that were modified and untracked files that are not ignored. Builds must never change SDK code\nor config; `mcu -v build` runs this check after the build.\n\nexamples:\n  mcu check-clean wch\n  mcu check-clean\n' ;;
    ide) printf '\nRuns clion/mcu-ide (CLion project generator, maintained with the CLion setup).\n\nexample:\n  mcu ide new stm32 myproj blackpill-f411\n' ;;
    help) printf '\nexample:\n  mcu help clean\n' ;;
  esac
  return 0
}

# help [command]
cmd_help() {
  if [[ $# -eq 0 ]]; then
    help_overview
  else
    help_command "$1"
  fi
}
