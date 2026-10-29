# `<sdk>/sdk.sh` plugin contract (v2)

Every SDK in this workspace is a directory `<sdk>/` with a plugin `<sdk>/sdk.sh` and an environment `<sdk>/env.sh`.
`../mcu` finds plugins by that file, sources them with **bash** in a subshell (cwd = the SDK dir), and owns
everything around them: argument parsing, build logs and summaries, clean/distclean, status, update orchestration,
selfcheck. A plugin only describes its SDK and does the SDK-specific work.

Start from [`lib/plugin-template.sh`](lib/plugin-template.sh). Validate with `./mcu check-plugins <sdk>`.
Reference plugins: [`stm32/sdk.sh`](stm32/sdk.sh) (CMake) and [`wch/sdk.sh`](wch/sdk.sh) (make, binary tools, update logic).

## Environment the plugin gets

| Var | Value |
|---|---|
| `MCU_ROOT` | the workspace (`$BOX/30_39_tools/32_embedded/32_01_mcu/tool`; SDK names there are symlinks, see README.md "Where this lives") |
| `SDK`, `SDK_DIR` | the SDK name, `$MCU_ROOT/<sdk>` |
| `MCU_OUT` | `$MCU_OUT_BASE/<sdk>` (default `$MCU_OUT_BASE/<sdk>`), build output root |
| `MCU_JOBS` | parallel jobs (default 16, `-j N`) |
| `MCU_YES`, `MCU_DRY_RUN`, `MCU_VERBOSE`, `MCU_QUIET` | global flags `-y -n -v -q` (use `upd_yes`, `trash_move`; they honor them) |
| `MCU_STRICT=1` | set during `mcu selfcheck` (PATH = `host/sysbin` only) |

Already loaded before `sdk.sh`: `lib/update.sh` (`upd_*`, default `sdk_rollback`/`sdk_prune`) and `lib/helpers.sh` (below).
The dispatcher's `log_info/log_warn/log_error/die` are also available.

## Variables

| Variable | Required | Meaning |
|---|---|---|
| `SDK_API=2` | yes | marks a v2 plugin (without it the plugin runs in v1 compat mode) |
| `SDK_DESC` | yes | one line for `mcu list` |
| `SDK_SELFCHECK` | yes* | `"<target> [example]"` built by `mcu selfcheck` under the strict PATH |
| `SDK_NO_SELFCHECK` | * | reason, for SDKs with nothing to build and no `sdk_selfcheck` (e.g. host) |
| `SDK_DEFAULT_EXAMPLE` | with `sdk_build` | example used when the user names none; mcu always passes it explicitly |
| `SDK_FLASH_TOOLS` | no | shown by `mcu info`, e.g. `"st-flash (default), openocd"` |

\* one of `SDK_SELFCHECK`, `sdk_selfcheck`, `SDK_NO_SELFCHECK`.

## Functions

Required:

| Function | Args | Must do |
|---|---|---|
| `sdk_env` | — | `source "$SDK_DIR/env.sh"`; runs before every other function |
| `sdk_doctor` | — | one `ok`/`MISSING`/`warn` line per tool (`doctor_tool`, `doctor_path`, `print_ok_missing`); non-zero when something required is missing |
| `sdk_versions` | — | installed versions read from git/binaries (`print_version`, `git_ref`, `tool_version`), never hard-coded |
| `sdk_targets` | — | (with `sdk_build`) `target  description` lines, owned boards first; first column = target name |
| `sdk_examples` | — | (with `sdk_build`) example names, first column = name |

Build and hardware (v2 signatures; mcu fills in the default example and strips `--`):

| Function | Args | Must do |
|---|---|---|
| `sdk_build` | `<target> <example> [extra...]` | build into `$(build_dir target example)` = `$MCU_OUT/<target>/<example>`, return the status. Do not log, list artifacts or time: mcu writes `build.log` there and prints OK/FAIL, duration and artifacts. **Never write inside the SDK tree** (build out of tree; `mcu check-clean` verifies) |
| `sdk_flash` | `<target> <example> [port\|probe]` | flash the last build with the tool for that board |
| `sdk_monitor` | `[port] [baud]` | serial console |

Optional hooks:

| Function | Purpose |
|---|---|
| `sdk_clean <target> <example>` | override the default clean (trash `$(build_dir target example)`) |
| `sdk_build_dir <target> <example>` | when the output dir is not `$MCU_OUT/<target>/<example>` |
| `sdk_artifacts <target> <example>` | artifact paths, when the default (`*.elf *.bin *.hex *.uf2 *.img *.kdimg *.dfu`, depth 3) is wrong |
| `sdk_build_outputs` | extra build-output dirs outside `$MCU_OUT/<target>` (e.g. an in-SDK `output/`) for `clean <sdk>` / `distclean` |
| `sdk_cache_dirs` | cache dirs (download cache, ccache...) for `mcu clean --cache <sdk>` |
| `sdk_clean_cache` | custom cache cleaning instead of `sdk_cache_dirs` |
| `sdk_info` | extra `label  value` lines for `mcu info` |
| `sdk_new <name> [target]` | template project into `$MCU_ROOT/projects/<name>` |
| `sdk_selfcheck` | strict-PATH check for SDKs with nothing to build (then leave `SDK_SELFCHECK` empty) |
| `sdk_update [version]` | see "Update / rollback contract" |
| `sdk_prune_families` | `stable-link\|glob` lines (relative to the SDK dir) for `mcu prune` |
| `sdk_rollback` | override the default replay of `.update-state` |

Private functions are named `<sdk>_<verb>_<noun>` (e.g. `wch_build_ch32fun`), small, with `local` variables and a
one-line comment saying what they are for.

## Helper API (`lib/helpers.sh`)

| Helper | Use |
|---|---|
| `print_ok_missing <label> <0\|1> [detail]`, `print_warn <label> [detail]` | doctor lines |
| `doctor_tool <label> <cmd> [version args]`, `doctor_path <label> <path> [detail]`, `doctor_git <label> <dir>` | doctor checks with versions |
| `print_version <component> <version>`, `tool_version <cmd> [args]` | version lines |
| `has_tool <cmd>`, `require_tool <cmd> [hint]` | tool presence (returns 1 with a hint) |
| `git_ref <dir>`, `git_head <dir> [len]`, `git_remote_head <dir> [len]`, `latest_github_release <owner/repo>` | versions |
| `target_names`, `require_target <target>` | validate a target against `sdk_targets` |
| `build_dir <target> [example]` | `$MCU_OUT/<target>/<example>` (`/` in example becomes `_`) |
| `cmake_build <src> <dir> [cmake args]` | configure once with Ninja, build with `$MCU_JOBS` |
| `make_build <dir> [make args]` | `make -C dir -j$MCU_JOBS` |
| `copy_artifacts <src> <out> <glob...>` | copy build products into the output dir |
| `list_artifacts <dir> [glob...]` | `path<TAB>bytes` of firmware files |
| `trash_move <path> [name]` | move to the Trash (per-volume `.Trash-<uid>` or `~/.local/share/Trash`, with `.trashinfo`); honors `--dry-run` |

Update side (`lib/update.sh`): `upd_step`, `upd_report`, `upd_dry_hint`, `upd_fetch`, `upd_unpack`, `upd_flip`,
`upd_rec`, `upd_git_rec`, `upd_patch`, `upd_gh_latest`, `upd_gh_asset`, `upd_git_newest_tag`, `upd_pypi_latest`,
`upd_xpack_info`, `upd_install_xpack`, `upd_yes`.

## Rules (checked by `mcu check-plugins`)

- No `rm`: move things with `trash_move` (v2 plugins must not write to `~/.local/share/Trash` by hand).
- No OS tools by absolute path (`/usr/bin/...`, `/bin/...`).
- `set -u` and `set -e` safe (mcu runs builds with errexit on), no `cd` outside `$SDK_DIR`/`$MCU_OUT`, no sudo, never edit shell rc files.
- Builds never modify the SDK tree: no tracked file changes, no untracked files (`mcu check-clean <sdk>`; `mcu -v build` runs it).

## Self-contained rule (no OS packages)

- `<sdk>/env.sh` starts by sourcing `../host/env.sh` (git, make, gcc/g++, tar, patch, wget, m4, pkg-config from Yocto
  buildtools 6.0.4; cmake 4.4.4, ninja 1.13.2, uv 0.13.5, uv-managed CPython 3.12/3.13). Cross toolchains live in the
  SDK dir; the shared Arm one is `tools/toolchains/arm-gnu-toolchain`. Works from bash and zsh: take the file
  path from `BASH_SOURCE[0]` in bash and `$0` in zsh (see `wch/env.sh`).
- A plugin dir that is a symlink into another ID (`freertos` -> 32_04_freertos, `zephyr` -> 32_03_zephyr) must find
  its siblings through its physical path (`cd -P "$(dirname "$f")/.." && pwd -P`), or `..` resolves to `mcu/` and the
  plugin silently uses mcu's host/tools instead of its own copy. Both env.sh files do this since 2026-10-24.
- Keep tool locations in one block of `env.sh` that names only stable symlinks (`toolchain/xpack-riscv-none-elf-gcc`,
  `wlink`, ...), never versioned dirs, so updates flip links and never edit env.sh. Framework code finds an SDK's
  environment only through `sdk_env_file` (lib/session.sh), the hook for pinned versions later.
- Never call `/usr/bin/...`. Create venvs with `uv venv --python 3.12` (`UV_PYTHON_PREFERENCE=only-managed`), always
  `uv python install --no-bin`.
- Host binaries you build must link only glibc and libraries inside mcu/ (`./mcu libcheck <sdk>`).
- `./mcu selfcheck <sdk>` builds `SDK_SELFCHECK` with `PATH=host/sysbin` (the OS base utilities in `host/sysbin.list`). It must pass.
- Allowed from the OS: kernel, glibc, the utilities in `host/sysbin.list`, `/etc/ssl` CA certificates, udev rules.

## Update / rollback contract (`mcu update`, `mcu outdated`, `mcu rollback`, `mcu prune`)

- `UPD_RC=0` at the top of `sdk_update`, `return "$UPD_RC"` at the end, `upd_dry_hint` before it.
- Per component: `upd_step <component> <installed> <latest> <installer fn> [args]`. It prints `installed -> latest`,
  records the row for the summary table and runs the installer only when applying (`--yes` / `MCU_YES=1`, not `--dry-run`).
  A failing installer sets `UPD_RC=1`. Components that cannot be automated: `upd_report` only.
- Binary tools: `upd_fetch url dest [url:<sha file>|hex:<sha256>]` (sha256 verified; unpublished sums are computed and
  recorded in `<sdk>/.update-sha256`), `upd_unpack`, install **side by side** in `<name>-<version>/`, then flip the
  stable symlink with `upd_flip` (atomic). `upd_install_xpack` handles xPack tools.
- Record how to undo before changing: `upd_rec <kind> <path> <previous> [extra]` (`upd_flip`, `upd_git_rec` do it).
  Kinds: `link git pip west repo arduino file`. Never `rm`; overwritten files are copied to `*.bak` first (kind `file`).

`mcu update <sdk> --yes` then runs the post-checks in `UPDATE_POST_CHECKS` (lib/update-cmd.sh: patch-rpath,
selfcheck of the SDK and of `sdks_to_recheck <sdk>`, libcheck). Any failure replays the recorded state in reverse
(automatic rollback). On success the state is kept in `<sdk>/.update-state` and `<sdk>/VERSIONS.md` is regenerated.
`mcu outdated` is the dry run of every SDK, printing only components that are not current (cached for `mcu status`).
`mcu rollback <sdk>` replays `.update-state` by hand; `mcu prune <sdk|all> --yes` trashes versions older than the previous one.

## v1 compat mode

Plugins without `SDK_API=2` keep working unchanged: they get the user's arguments verbatim
(`sdk_build <target> [example] [-- extra]`), their own `sdk_clean`, and the framework log is named `mcu-build.log`
(v1 plugins may write their own `build.log`). `check-plugins` reports them as `contract v1 (compat mode)`.

### Migrating a v1 plugin to v2 (checklist)

1. Header `# shellcheck shell=bash disable=SC2034,SC2153`, then `SDK_API=2`, `SDK_DESC`, `SDK_SELFCHECK` (or
   `SDK_NO_SELFCHECK`), `SDK_DEFAULT_EXAMPLE`, `SDK_FLASH_TOOLS`.
2. `sdk_build`/`sdk_flash` take `<target> <example> [...]` (no defaulting, no `--` handling). Remove logging, `ls`
   of artifacts and timing; add `sdk_artifacts` only if the default listing is wrong.
3. Delete `sdk_clean` when it only trashed `$MCU_OUT/<target>/<example>`; otherwise keep it using `trash_move`.
   List outputs outside `$MCU_OUT/<target>` in `sdk_build_outputs`, caches in `sdk_cache_dirs`.
4. Replace hand-rolled `_chk`, `ok/MISSING` echo lines, `mv ... ~/.local/share/Trash`, raw `curl` GitHub calls with the helpers.
5. Build out of tree; `mcu check-clean <sdk>` must stay CLEAN after a build.
6. env.sh: zsh/bash file detection without `${(%):-%x}`, one component-location block.
7. Check: `./mcu check-plugins <sdk>`, `./mcu lint`, `./mcu fmt --check`, `./mcu doctor <sdk>`,
   `./mcu rebuild <sdk> <target>`, `./mcu selfcheck <sdk>`, `./mcu check-clean <sdk>`, `./mcu test`.
