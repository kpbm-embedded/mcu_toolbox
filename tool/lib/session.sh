# shellcheck shell=bash
# session.sh - commands that work with hardware or an interactive environment:
# flash, monitor, shell, run, env and new (project from template).

# The environment file of <sdk>. The one place the framework resolves it.
# Hook: version management can return a generated env with pinned component paths here instead.
sdk_env_file() {
  local sdk="$1"
  printf '%s/%s/env.sh\n' "$MCU_ROOT" "$sdk"
}

# Stop unless <sdk> has an env.sh.
require_env_file() {
  local sdk="$1"
  require_sdk "$sdk"
  [[ -f "$(sdk_env_file "$sdk")" ]] || die "$sdk has no env.sh"
}

# flash <sdk> <target> [example] [port|probe]
cmd_flash() {
  local sdk="${1:-}" target="${2:-}" example port
  [[ $# -ge 2 && $# -le 4 ]] || die_usage flash "need <sdk> <target> [example] [port|probe]"
  require_sdk "$sdk"
  if ! plugin_is_v2 "$sdk"; then
    shift
    plugin_call "$sdk" sdk_flash "$@"
    return
  fi
  example="$(build_pick_example "$sdk" "${3:-}")"
  port="${4:-}"
  if is_dry_run; then
    echo "would flash $sdk $target $example ${port:+via $port}"
    return 0
  fi
  plugin_call "$sdk" sdk_flash "$target" "$example" ${port:+"$port"}
}

# monitor <sdk> [port] [baud]
cmd_monitor() {
  local sdk="${1:-}"
  [[ $# -ge 1 && $# -le 3 ]] || die_usage monitor "need <sdk> [port] [baud]"
  require_sdk "$sdk"
  shift
  plugin_call "$sdk" sdk_monitor "$@"
}

# new <sdk> <name> [target]: new project in projects/<name> from templates/<sdk>/{minimal,minimal-*,*-minimal} (the one whose
# targets include <target>; instantiate.py "auto"). SDKs without templates fall back to the plugin's sdk_new.
cmd_new() {
  local sdk="${1:-}" name="${2:-}" target="${3:-}" dest py
  [[ $# -ge 2 && $# -le 3 ]] || die_usage new "need <sdk> <name> [target]"
  require_sdk "$sdk"
  [[ "$name" =~ ^[A-Za-z0-9][A-Za-z0-9._-]*$ ]] || die_usage new "bad project name '$name'"
  if ! compgen -G "$MCU_ROOT/templates/$sdk/*minimal*/template.toml" >/dev/null; then
    if is_dry_run; then
      echo "would create project $name from the $sdk plugin template"
      return 0
    fi
    shift
    plugin_call "$sdk" sdk_new "$@"
    return
  fi
  dest="$MCU_ROOT/projects/$name"
  [[ ! -e "$dest" ]] || die "$dest exists"
  if is_dry_run; then
    echo "would create project $name from templates/$sdk (${target:-default target}) in $dest"
    return 0
  fi
  py="$MCU_ROOT/host/python/cpython-3.12-linux-x86_64-gnu/bin/python3"
  [[ -x "$py" ]] || py=python3
  "$py" -I "$MCU_ROOT/templates/tools/instantiate.py" "$sdk" auto "$dest" --name "$name" ${target:+--target "$target"} ||
    die "could not create $dest"
  echo "created $dest (README.md there has the build commands; templates/tools/check.sh builds the same way)"
}

# shell <sdk>: interactive bash with the SDK environment.
cmd_shell() {
  local sdk="${1:-}" env_file
  [[ $# -eq 1 ]] || die_usage shell "need one <sdk>"
  require_env_file "$sdk"
  env_file="$(sdk_env_file "$sdk")"
  log_info "entering $sdk environment (exit to leave)"
  # shellcheck disable=SC2016  # $PS1 expands in the new shell, not here
  exec bash --rcfile <(printf 'source ~/.bashrc 2>/dev/null\nsource %q\nPS1="(mcu:%s) $PS1"\n' "$env_file" "$sdk") -i
}

# run <sdk> [--] <cmd...>: one command inside the SDK environment.
cmd_run() {
  local sdk="${1:-}" env_file
  [[ $# -ge 1 ]] || die_usage run "need <sdk> -- <cmd...>"
  require_env_file "$sdk"
  shift
  [[ "${1:-}" == -- ]] && shift
  [[ $# -ge 1 ]] || die_usage run "no command given"
  env_file="$(sdk_env_file "$sdk")"
  (
    # shellcheck source=/dev/null
    source "$env_file" && exec "$@"
  )
}

# env <sdk>: print the line that loads the SDK environment into the current shell.
cmd_env() {
  local sdk="${1:-}"
  [[ $# -eq 1 ]] || die_usage env "need one <sdk>"
  require_env_file "$sdk"
  printf "source '%s'\n" "$(sdk_env_file "$sdk")"
}

# ide ...: CLion project tooling, maintained in clion/ (see clion/README.md).
cmd_ide() {
  local tool="$MCU_ROOT/clion/mcu-ide"
  [[ -x "$tool" ]] || die "clion/mcu-ide is not installed"
  exec "$tool" "$@"
}
