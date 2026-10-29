# mcu workspace

One command, `./mcu`, for every microcontroller SDK and toolchain in this directory: STM32Cube, FreeRTOS, Zephyr,
ESP-IDF, Pico SDK, WCH CH32V, Puya PY32, AVR/CH552, Bouffalo, Kendryte K210/K230, plus the shared host layer and
debug/flash tools. Everything is self-contained: no OS packages beyond the kernel, glibc and base utilities.

## Quick start

```sh
cd $BOX/30_39_tools/32_embedded/32_01_mcu/tool      # or just `mcu ...` after `box shell install --apply`
./mcu list                               # SDKs
./mcu info stm32                         # targets, examples, flash tools, docs of one SDK
./mcu build stm32 blackpill-f411         # build the default example
./mcu flash stm32 blackpill-f411         # flash it (st-flash by default)
./mcu monitor stm32 /dev/ttyUSB0
./mcu status                             # one table for everything
```

Builds go to `$MCU_OUT_BASE/<sdk>/<target>/<example>/` (`MCU_OUT_BASE`), with `build.log` next to the
artifacts. Every build ends with a summary:

```
OK  wch ch32v003 blink  (0.5s)
  692 B      $MCU_OUT_BASE/wch/ch32v003/blink/blink.bin
  47.0 KiB   $MCU_OUT_BASE/wch/ch32v003/blink/blink.elf
  log: $MCU_OUT_BASE/wch/ch32v003/blink/build.log
```

## Global flags

Flags work anywhere before a `--` (and the environment variables still work):

| Flag | Env | Effect |
|---|---|---|
| `-y`, `--yes` | `MCU_YES=1` | apply updates, confirm clean/distclean without asking |
| `-n`, `--dry-run` | `MCU_DRY_RUN=1` | show what would change, change nothing (wins over `--yes`) |
| `-v`, `--verbose` | `MCU_VERBOSE=1` | debug lines; `build` also runs `check-clean` afterwards |
| `-q`, `--quiet` | `MCU_QUIET=1` | build output only in the log; the summary (and the log tail on failure) is printed |
| `-j N`, `--jobs N` | `MCU_JOBS=N` | parallel jobs (default 16) |
| `--no-color` | `NO_COLOR=1`, `MCU_COLOR=never` | plain output (colors are only used on a terminal anyway) |
| `-h`, `--help` | | help for the command |

`MCU_OUT_BASE` moves the build output; `MCU_TRASH` overrides the Trash directory (used by the tests).

## Commands

`./mcu help` lists them, `./mcu help <command>` (or `./mcu <command> --help`) explains one with examples.

Build and flash

| Command | Example |
|---|---|
| `build <sdk> <target> [example] [-- args]` | `./mcu build zephyr rpi_pico samples/hello_world -- -DCONFIG_DEBUG=y` |
| `rebuild <sdk> <target> [example]` | `./mcu rebuild wch ch32v003 blink` (clean, then build) |
| `flash <sdk> <target> [example] [port\|probe]` | `./mcu flash wch ch32v003 blink wlink` |
| `monitor <sdk> [port] [baud]` | `./mcu monitor esp-idf /dev/ttyACM0` |
| `clean <sdk> <target> [example]` | `./mcu clean stm32 blackpill-f411` (that build dir, no question) |
| `clean <sdk>` / `clean all` | `./mcu clean stm32 --yes` (every per-target build dir of the SDK / of all SDKs) |
| `clean --cache <sdk\|all>` | `./mcu clean --cache wch -n` (download caches etc., when the plugin defines them) |
| `distclean <sdk\|all>` | `./mcu distclean wch --yes` (builds + selfcheck output + status records) |
| `logs <sdk> [target] [example] [--lines N]` | `./mcu logs stm32` (newest build or selfcheck log, path + tail) |

Information

| Command | Example |
|---|---|
| `list` | `./mcu list` |
| `info <sdk>` | `./mcu info wch` |
| `status [sdk] [--refresh]` | `./mcu status` (versions, last build, selfcheck, updates, disk use) |
| `doctor [sdk\|all]` | `./mcu doctor pico` |
| `versions [sdk\|all]` | `./mcu versions zephyr` |
| `targets <sdk>` / `examples <sdk>` | `./mcu targets k230` |
| `where` | `./mcu where` (tool dir, build output, state, selfcheck logs, Trash; read-only) |

Maintenance

| Command | Example |
|---|---|
| `update <sdk\|all> [version]` | `./mcu update wch` (dry run), `./mcu update wch --yes` (apply, verify, auto-rollback) |
| `outdated` | `./mcu outdated` (components with a newer version, all SDKs) |
| `rollback <sdk>` | `./mcu rollback wch` |
| `prune [sdk\|all]` | `./mcu prune host --yes` |
| `selfcheck [sdk\|all]` | `./mcu selfcheck stm32` (build with PATH = OS base utilities only) |
| `libcheck [sdk\|all]` | `./mcu libcheck wch` |
| `check-plugins [sdk\|all]` | `./mcu check-plugins` |
| `check-clean [sdk\|all]` | `./mcu check-clean wch` (SDK git checkouts unchanged by builds) |

Environment

| Command | Example |
|---|---|
| `shell <sdk>` | `./mcu shell wch` |
| `run <sdk> -- <cmd...>` | `./mcu run stm32 -- arm-none-eabi-gcc --version` |
| `env <sdk>` | `eval "$(./mcu env zephyr)"` |
| `new <sdk> <name> [target]` | `./mcu new stm32 myproj` |
| `ide new <sdk> <name> [target] [--source DIR]` | CLion projects, see [clion/README.md](clion/README.md) |

Development of the scripts themselves

| Command | Example |
|---|---|
| `lint [--all]` | `./mcu lint` (shellcheck; `--all` includes not-yet-migrated plugins) |
| `fmt [--check] [--all]` | `./mcu fmt --check` (shfmt `-i 2 -ci -bn`) |
| `test [bats args]` | `./mcu test` |
| `devtools [install]` | `./mcu devtools install` (pinned, sha256-verified shellcheck, shfmt, bats-core) |

`install`, `use`, `graph`, `deps`, `rdeps`, `check-deps` and `lock` are reserved for version management (exit 3: not implemented).

## Nothing is deleted

`clean`, `distclean`, `prune` and `clean --cache` move things to the Trash: the per-volume `<build disk>/.Trash-<uid>`
for the build disk, `~/.local/share/Trash` otherwise, with `.trashinfo` files so a file manager can restore them.
Bulk forms ask first (refuse without a terminal) unless `--yes`; `--dry-run` only lists. Bulk clean only selects the
per-target build dirs; SDK sources that live on the build disk (e.g. `k230/sdk`) are never selected.

## Update and rollback

`./mcu update <sdk>` is a dry run listing every component as `installed -> latest`. With `--yes` it installs side by
side (`<name>-<version>/` behind a stable symlink), then runs patch-rpath, selfcheck and libcheck; any failure rolls back
automatically. The applied state is kept in `<sdk>/.update-state` (`./mcu rollback <sdk>` undoes it) and the versions
in `<sdk>/VERSIONS.md`. `./mcu update all` continues past failures and ends with a summary table; `./mcu outdated`
shows only what is not current.

## The self-contained rule

- Every `<sdk>/env.sh` sources `host/env.sh` (Yocto buildtools, cmake, ninja, uv, managed CPython); cross toolchains
  live inside the SDK dirs. See [host/README.md](host/README.md).
- Nothing calls `/usr/bin/...`; Python comes from `uv` (`uv python install --no-bin`).
- Host binaries link only glibc and libraries inside mcu/ (`./mcu libcheck`).
- `./mcu selfcheck` proves each SDK builds with `PATH` limited to the OS base utilities in `host/sysbin.list`.
- Builds never change SDK sources or config (`./mcu check-clean`).

## Layout

```
mcu                  the dispatcher: global flags, command table, help
lib/core.sh          settings, logging (colors on a terminal only), prompts, formatters
lib/plugin.sh        plugin discovery, loading in a subshell, check-plugins
lib/helpers.sh       helper API for plugins (doctor lines, cmake/make builds, artifacts, trash_move...)
lib/build.sh         build, rebuild, clean, distclean, logs
lib/check.sh         doctor, selfcheck, libcheck, check-clean, host/sysbin
lib/update.sh        plugin-side update API (upd_*), default rollback/prune (host/update-lib.sh is a shim)
lib/update-cmd.sh    update, outdated, rollback, prune
lib/query.sh         list, info, status, versions, targets, examples, completion words
lib/session.sh       flash, monitor, shell, run, env, new, ide
lib/dev.sh           devtools, lint, fmt, test
lib/help.sh          help texts
lib/plugin-template.sh  starting point for a new plugin
completion/          mcu.bash, _mcu (zsh)
tests/               bats suites, fixtures/fake (test SDK), bats -> bats-core-<ver>
host/                shared host layer (+ devtools/ for shellcheck and shfmt)
<sdk>/sdk.sh         one plugin per SDK, contract in PLUGIN.md
clion/               CLion integration (clion/README.md)
```

### Where this lives (since the 2026-10-24 reorganisation)

`mcu` is `32_01_mcu/tool/mcu` in the `32_embedded` category (`$BOX/30_39_tools/32_embedded`).
The SDK, host, tools, clion, templates, examples and projects names above are symlinks inside `tool/`, so every
`$MCU_ROOT/<name>` path still works:

| `tool/<name>` | real folder |
|---|---|
| `avr bouffalo esp-idf host k210 k230 pico py32 stm32 tools wch` | `32_01_mcu/toolchain/<name>` |
| `freertos` | `32_04_freertos/toolchain/freertos` (that ID's own, self-contained copy of host/tools/stm32 is used) |
| `zephyr` | `32_03_zephyr/toolchain/zephyr` (uses 32_03_zephyr's own host copy) |
| `clion` | `32_01_mcu/ide/clion` (`mcu-ide`, CLion project generator) |
| `templates`, `examples`, `projects` | `32_01_mcu/{templates,examples,projects}`; `examples/{freertos,zephyr}` and the `freertos-blinky-f411` / `zephyr-blinky-blackpill` projects are symlinks into 32_04 / 32_03 |

mcu may point into 32_03_zephyr and 32_04_freertos; those IDs never point back (they build on their own with
`scripts/app`, see their README.md). Build output: `MCU_OUT_BASE` = `$MCU_OUT_BASE`
(CLion projects: `.../build/projects/<name>/cmake-build-<profile>`).

## Shell completion

Completes commands, SDK names, targets and examples (cached per SDK, refreshed when its sdk.sh changes).
mcu never edits your rc files; add the lines yourself (`box shell install --apply` only gives command-name completion):

```sh
# bash (~/.bashrc)
source $BOX/30_39_tools/32_embedded/32_01_mcu/tool/completion/mcu.bash

# zsh (~/.zshrc), before compinit
fpath=($BOX/30_39_tools/32_embedded/32_01_mcu/tool/completion $fpath)
autoload -Uz compinit && compinit
compdef _mcu mcu ./mcu
```

## Writing or migrating a plugin

See [PLUGIN.md](PLUGIN.md): contract v2, the helper API, the rules `check-plugins` enforces and a migration checklist.

## Scripts

Every script prints its usage with `-h`/`--help`. Exit codes: 0 ok, 1 failure, 2 usage error, 3 not implemented (reserved commands).

- `mcu` - the dispatcher: global flags, the command table, help; the logic is in `lib/`.
- `lib/core.sh` - settings (`MCU_*`), logging, `die`/`die_usage`, dry-run/confirm, formatters.
- `lib/helpers.sh` - target validation, CMake/make builds, artifact listing, moving things to the Trash.
- `lib/plugin.sh` - loading and calling SDK plugins (`<sdk>/sdk.sh`).
- `lib/build.sh` - `build`, `rebuild`, `flash`, `monitor`, `clean`, `distclean`, `logs`.
- `lib/check.sh` - `doctor`, `selfcheck`, `libcheck`, `check-plugins`, `check-clean`.
- `lib/update-cmd.sh`, `lib/update.sh` - `update`/`outdated`/`rollback`/`prune` and the plugin-side update API.
- `lib/query.sh` - `list`, `info`, `status`, `versions`, `targets`, `examples`, `where`.
- `lib/session.sh` - `shell`, `run`, `env`, `new`, `ide`.
- `lib/dev.sh` - `lint`, `fmt`, `test`, `devtools`.
- `lib/help.sh` - `mcu help <command>` texts.
- `lib/plugin-template.sh` - starting point for a new plugin (copy to `<sdk>/sdk.sh`).
- `tests/*.bats` (+ `helpers.bash`, `fixtures/fake/`) - the test suite; run `mcu test`.
- `completion/mcu.bash`, `completion/_mcu` - bash/zsh completion.
- `../templates/tools/check.sh` - build every example/template of SDKs for every target, print a table.
- `../templates/tools/build/<sdk>.sh` - per-SDK build adapters used by `check.sh` (`tpl_build`).
- `../templates/tools/instantiate.py` - create a project from a template (used by `mcu new`).
- `../templates/tools/index.py` - regenerate `examples/README.md` and `templates/README.md`.
- `../templates/tools/tomlget.py` - print one key of a TOML file (used by `check.sh`).
