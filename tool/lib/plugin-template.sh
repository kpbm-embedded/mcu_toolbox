# shellcheck shell=bash disable=SC2034,SC2153
# <sdk> plugin (contract v2, see ../PLUGIN.md). Copy to <sdk>/sdk.sh and replace every "example" part.
# Sourced by ../mcu in a subshell with cwd=$SDK_DIR; SDK, SDK_DIR, MCU_ROOT, MCU_OUT, MCU_JOBS are exported.
# lib/update.sh (upd_*) and lib/helpers.sh (doctor_tool, cmake_build, trash_move, ...) are already loaded.
# SC2034/SC2153: the SDK_* variables are read by ../mcu; MCU_* are exported by it.

SDK_API=2
SDK_DESC="Example SDK: one line shown by mcu list"
SDK_SELFCHECK="example-board hello"                # "<target> [example]" built by mcu selfcheck
SDK_DEFAULT_EXAMPLE=hello                          # used when the user names no example
SDK_FLASH_TOOLS="example-flash (default), openocd" # shown by mcu info

sdk_env() {
  # shellcheck source=/dev/null
  source "$SDK_DIR/env.sh"
}

# Required: one ok/MISSING line per tool; non-zero when something required is missing.
sdk_doctor() {
  local rc=0
  doctor_tool gcc example-gcc || rc=1
  doctor_tool cmake cmake || rc=1
  doctor_path "sdk tree" "$SDK_DIR/sdk" "$(git_ref "$SDK_DIR/sdk")" || rc=1
  return "$rc"
}

# Required: installed versions, read from the tools (never hard-coded).
sdk_versions() {
  print_version sdk "$(git_ref "$SDK_DIR/sdk")"
  print_version example-gcc "$(tool_version example-gcc --version)"
}

# Required with sdk_build: "<target>  <description>" lines, owned boards first.
sdk_targets() {
  echo "example-board  Example board (owned)"
}

# Required with sdk_build: the example names sdk_build accepts.
sdk_examples() {
  echo "hello"
}

# sdk_build <target> <example> [extra args]: build into $(build_dir target example).
# mcu logs the output to build.log there and prints the artifacts; just build and return the status.
sdk_build() {
  local target="$1" example="$2"
  shift 2
  require_target "$target" || return 1
  cmake_build "$SDK_DIR/examples/$example" "$(build_dir "$target" "$example")" "$@"
}

# sdk_flash <target> <example> [port|probe]
sdk_flash() {
  local target="$1" example="$2" probe="${3:-example-flash}"
  example-flash --probe "$probe" "$(build_dir "$target" "$example")/$example.bin"
}

# sdk_monitor [port] [baud]
sdk_monitor() {
  python -m serial.tools.miniterm "${1:-/dev/ttyUSB0}" "${2:-115200}"
}

# Optional hooks (delete what does not apply):
#   sdk_clean <target> <example>    override the default (trash $(build_dir target example))
#   sdk_build_dir <target> <example> output dir when it is not $MCU_OUT/<target>/<example>
#   sdk_artifacts <target> <example> artifact paths when the default (*.elf *.bin *.hex *.uf2 ...) is wrong
#   sdk_build_outputs               extra dirs with build output outside $MCU_OUT (clean <sdk>, distclean)
#   sdk_cache_dirs                  cache dirs for `mcu clean --cache` (download caches, ccache...)
#   sdk_clean_cache                 custom cache cleaning instead of sdk_cache_dirs
#   sdk_info                        extra "label  value" lines for `mcu info`
#   sdk_new <name> [target]         project template into $MCU_ROOT/projects/<name>
#   sdk_selfcheck                   for SDKs with nothing to build (then leave SDK_SELFCHECK empty)
#   sdk_prune_families              "stable-link|glob" lines for `mcu prune`
#   sdk_rollback                    override the default replay of .update-state

# sdk_update [version]: one upd_step per component (see PLUGIN.md "Update / rollback contract").
sdk_update() {
  UPD_RC=0
  local current newest
  current="$(git_ref "$SDK_DIR/sdk")"
  newest="$(upd_git_newest_tag https://github.com/example/sdk '^v[0-9.]+$')"
  upd_step sdk "$current" "${1:-$newest}" example_checkout "${1:-$newest}"
  upd_dry_hint
  return "$UPD_RC"
}

# Installer for sdk_update: record the rollback state first, then change things.
example_checkout() {
  local tag="$1"
  upd_git_rec sdk
  git -C "$SDK_DIR/sdk" fetch -q --depth 1 origin "refs/tags/$tag:refs/tags/$tag" && git -C "$SDK_DIR/sdk" checkout -q "$tag"
}
