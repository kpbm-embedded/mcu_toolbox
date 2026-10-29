# 32_01_mcu - mcu (one command for every MCU SDK)

`mcu` is a single launcher for the microcontroller SDKs and toolchains in this workspace: build, flash, monitor, update
and self-check STM32, ESP32, Pico, WCH CH32V, PY32, AVR/CH552, Bouffalo, Kendryte K210/K230, FreeRTOS and Zephyr from
one place. Everything is self-contained (no OS packages beyond kernel, glibc and base utilities).

Status: **ready** for building (2026-10-29: bats 59/59; `check.sh` bare 13 examples + 12 templates, all run in
qemu/Renode and pass; pico/stm32/wch/freertos builds pass); **flashing is untested on hardware** (no board attached).
New in 2026-10-29: the `bare` SDK (bare-metal Cortex-M/RISC-V with qemu/Renode test runs, Rust and Zig via FFI),
`mcu setup` (pinned, checksum-verified installs from `config/pins.tsv`), a git repo (`mcu`) with sources only.

## What's included

| Component | Version | Notes |
|---|---|---|
| mcu dispatcher + plugins | contract v2 | `tool/mcu`, one `sdk.sh` plugin per SDK (`tool/PLUGIN.md`) |
| host layer | buildtools-extended 6.0.4, cmake 4.4.4, ninja 1.13.2, uv 0.13.5, CPython 3.8/3.12/3.13 | shared by all SDKs |
| stm32 | Cube F4 v1.28.3, F1 v1.8.7, G4 v1.6.3, H7 v1.13.0; Arm GNU 15.2.rel1 | flash: st-flash, openocd, pyocd, probe-rs, dfu |
| pico | pico-sdk 2.3.1, picotool 2.3.1 | RP2040 / RP2350 |
| esp-idf | v6.1 | ESP32, S3, C3, C6, P4 |
| wch | xPack gcc 15.2.0, ch32fun, wlink 0.1.3, wchisp 0.3.0 | CH32V |
| py32 | py32f0-template, pyocd 0.46.0 | Puya PY32F0xx |
| avr | arduino-cli 1.5.1, avr-gcc 7.3.0, ATTinyCore 1.5.2, CH55xDuino 0.0.26 | Nano, Mega, ATtiny85, CH552 |
| bouffalo | bouffalo_sdk v2.3.37, Xuantie gcc 10.4.0 | BL616 / BL702 / BL808 |
| k210 / k230 | standalone SDK V0.5.7, k230_rtos_sdk v0.6-94 | kflash 1.1.6, k230-flash 1.4.0 |
| bare | Arm GNU 15.2.rel1, xPack RISC-V GCC 15.2.0-1, xPack QEMU Arm/RISC-V 9.2.4-1, Renode 1.17.0 | `toolchain/bare`: shared CMake toolchain files, 7 chip layers (startup + linker script + console), `run/firmware.sh` |
| freertos, zephyr | FreeRTOS-Kernel V11.3.2, Zephyr v4.4.2 / SDK 1.0.1 | live in `32_04_freertos` and `32_03_zephyr`; `mcu` reaches them by symlink (`mcu setup links`) |
| debug tools | pyocd 0.46.0, probe-rs 0.32.0, xPack OpenOCD 0.12.0, st-flash 1.9.0 | `tool/tools` |
| CLion integration | CLion 2026.2 | `ide/clion` (`mcu ide new`) |
| vendor apps | STM32CubeMX, CubeProgrammer, JBDTools (Windows exe), J-Link firmware | `vendor_apps/`, not used by `mcu` |

Versions: `mcu versions all`.

## Quick start

```sh
mcu setup install core bare pico            # fresh clone: pinned toolchains (sha256/commit checked; see "Setup" below)
mcu devtools install                        # shellcheck, shfmt, bats (pinned) for `mcu test`
box shell install --apply                   # once per machine: $BOX_* variables + PATH shims (incl. `mcu`) in every new shell
mcu list                                    # the SDKs (same as $BOX_MCU/mcu or ./mcu inside tool/)
mcu doctor stm32                            # are the tools present?
mcu build stm32 blackpill-f411              # build the default example
mcu flash stm32 blackpill-f411              # flash it (board attached)
mcu where                                   # tool dir, build output, state, Trash
```

Without the Box shell integration, run it by path: `cd $BOX/30_39_tools/32_embedded/32_01_mcu/tool && ./mcu list`
(or `box run mcu list` from anywhere).

The build ends with a summary (size of `.bin`/`.elf` and the path of `build.log`). Start with `./mcu info <sdk>` to see
targets, examples and flash tools of one SDK.

## Commands

Run from `tool/` as `./mcu <command>`. `./mcu help <command>` gives details and examples. Global flags work anywhere before `--`:
`-y/--yes`, `-n/--dry-run`, `-v/--verbose`, `-q/--quiet`, `-j N/--jobs N`, `--no-color`, `-h/--help`.

| Command | What it does | Example |
|---|---|---|
| `build <sdk> <target> [example] [-- args]` | build into `$MCU_OUT_BASE/<sdk>/<target>/<example>`, log + summary | `./mcu build wch ch32v003 blink` |
| `rebuild <sdk> <target> [example]` | clean that build, then build | `./mcu rebuild wch ch32v003 blink` |
| `flash <sdk> <target> [example] [port\|probe]` | flash the last build | `./mcu flash wch ch32v003 blink wlink` |
| `monitor <sdk> [port] [baud]` | serial console | `./mcu monitor esp-idf /dev/ttyACM0` |
| `clean <sdk> [target [example]] \| all \| --cache <sdk\|all>` | move build outputs (or caches) to the Trash | `./mcu clean stm32 blackpill-f411` |
| `distclean <sdk\|all>` | builds + selfcheck outputs + state of the SDK to the Trash | `./mcu distclean wch --yes` |
| `logs <sdk> [target] [example] [--lines N]` | path and tail of the last build/selfcheck log | `./mcu logs stm32` |
| `list` | SDKs with descriptions | `./mcu list` |
| `info <sdk>` | description, dirs, targets, examples, flash tools, docs | `./mcu info wch` |
| `status [sdk] [--refresh]` | versions, last build, selfcheck, updates, disk use | `./mcu status` |
| `doctor [sdk\|all]` | required tools present, with versions | `./mcu doctor pico` |
| `versions [sdk\|all]` | installed/pinned versions | `./mcu versions zephyr` |
| `targets <sdk>` | boards/targets the SDK can build | `./mcu targets k230` |
| `examples <sdk>` | examples the SDK can build | `./mcu examples stm32` |
| `where` | key paths: tool dir, build output, state, selfcheck logs, Trash (read-only) | `./mcu where` |
| `setup [list\|check\|install\|fetch\|latest\|links] [group\|name...]` | pinned installs from `config/pins.tsv` (sha256 / git commit checked) | `./mcu setup install core bare` |
| `update <sdk\|all> [version]` | dry run installed -> latest; `--yes` applies, verifies, rolls back on failure | `./mcu update wch --yes` |
| `outdated` | components with a newer version | `./mcu outdated` |
| `rollback <sdk>` | undo the last applied update | `./mcu rollback wch` |
| `prune [sdk\|all]` | versions older than the previous one (`--yes`: to the Trash) | `./mcu prune host --yes` |
| `selfcheck [sdk\|all]` | build SDK_SELFCHECK with PATH = OS base utilities only | `./mcu selfcheck stm32` |
| `libcheck [sdk\|all]` | host binaries loading non-glibc libraries from the OS | `./mcu libcheck wch` |
| `check-plugins [sdk\|all]` | validate plugins against `PLUGIN.md` | `./mcu check-plugins` |
| `check-clean [sdk\|all]` | SDK git checkouts unchanged by builds | `./mcu check-clean wch` |
| `shell <sdk>` | interactive shell with the SDK environment | `./mcu shell wch` |
| `run <sdk> -- <cmd...>` | run one command inside the SDK environment | `./mcu run stm32 -- arm-none-eabi-gcc --version` |
| `env <sdk>` | print the line that sources the SDK environment | `eval "$(./mcu env zephyr)"` |
| `new <sdk> <name> [target]` | new project in `projects/<name>` from `templates/<sdk>` | `./mcu new stm32 myproj` |
| `ide new <sdk> <name> [target] [--source DIR]` | CLion project, see `ide/clion/README.md` | `./mcu ide new stm32 myproj` |
| `lint [--all]`, `fmt [--check] [--all]`, `test [bats args]`, `devtools [install]` | develop the mcu scripts themselves | `./mcu test` |

`install use graph deps rdeps check-deps lock` are reserved for version management and not implemented (exit 3).

## Examples and templates

| What | Where | How |
|---|---|---|
| Examples (buildable demos) | `examples/` (catalogue: `examples/README.md`) | `./mcu examples <sdk>`, then `./mcu build <sdk> <target> <example>` |
| Templates (project skeletons) | `templates/` (catalogue: `templates/README.md`, format: `templates/SPEC.md`) | `./mcu new <sdk> <name> [target]` creates `projects/<name>` from the matching `*minimal` template |
| Support matrix per target | [EXAMPLES_MATRIX.md](EXAMPLES_MATRIX.md) | OK / n/a / GAP per SDK, target and example |
| Your projects | `projects/` | e.g. `stm32-blinky-f411`, `pico2-blink`, `esp32s3-blink`, `ch32v003-blink` |
| Build every example | `templates/tools/check.sh <sdk>` | prints a table; `--help` lists options; runs `needs.sh` (SKIP) and `test.sh` of a project |

New 2026-10-29 (`bare` SDK; all run in qemu/Renode by check.sh):

| Examples (14) | Templates (12, `templates/bare/`) |
|---|---|
| hello-chips (7 chips), bend-host-check (needs optional group bend), systick-irq, fault-decode, noinit-reboot, size-budget (+ test.sh budget), ringbuf-split (host ctest + target), q15-fir, bootloader-app, timer-irq-stm32, cxx-bare, mpu-stack-guard, rust-ffi, zig-ffi | minimal (7 chips, `mcu new bare`), irq-driven, fw-host-tests, own-startup, bootloader-app, nrf52840-uf2 (UF2 check), stm32-ll (Cube LL, no HAL), cxx-app, fault-handler, size-budget, rust-app, zig-app |

## Configuration and environment variables

| Variable | Default | Meaning |
|---|---|---|
| `MCU_OUT_BASE` | `$BOX_V3/30_39_tools/32_embedded/32_01_mcu/build` | build output base |
| `MCU_JOBS` | 16 | parallel jobs (`-j`) |
| `MCU_YES` | 0 | apply/confirm without asking (`-y`) |
| `MCU_DRY_RUN` | 0 | show, change nothing (`-n`; wins over `--yes`) |
| `MCU_VERBOSE` | 0 | debug lines; `build` also runs `check-clean` (`-v`) |
| `MCU_QUIET` | 0 | build output only in the log (`-q`) |
| `MCU_COLOR` | auto | `always` / `never`; `NO_COLOR=1` also works |
| `MCU_TRASH` | (Trash dir) | overrides the Trash directory (used by the tests) |
| `MCU_ROOT` | `tool/` | set by the dispatcher; do not set it |

## Where files go

| What | Where |
|---|---|
| Source, SDKs, toolchains | this ID: `toolchain/<sdk>` (symlinked into `tool/<sdk>`), `tool/`, `templates/`, `examples/`, `projects/` |
| Caches | per SDK under `toolchain/<sdk>`; `./mcu clean --cache <sdk>` removes them |
| Build output | `$BOX_V3/30_39_tools/32_embedded/32_01_mcu/build/<sdk>/<target>/<example>/` (idisk03, same ID path); CLion: `.../build/projects/<name>/cmake-build-<profile>` |
| Logs | `build.log` next to the artifacts; `./mcu logs <sdk>` |

`freertos` and `zephyr` are symlinks to `32_04_freertos/toolchain/freertos` and `32_03_zephyr/toolchain/zephyr` (those IDs never point back).

## Setup (fresh clone) and pins

`config/pins.tsv` lists every component `mcu setup` installs into `toolchain/`: archive URL + sha256 (from the
publisher's checksum file where one exists) or git URL + commit. Nothing is overwritten; downloads stay in
`cache/downloads` (point `SETUP_CACHE` at another copy to avoid downloading). `mcu setup list` shows the state.

| Group | Components |
|---|---|
| `core` | Yocto buildtools-extended 6.0.4, cmake 4.4.4, ninja 1.13.2, Arm GNU 15.2.rel1 |
| `bare` | xPack RISC-V GCC 15.2.0-1, xPack QEMU Arm + RISC-V 9.2.4-1, Renode 1.17.0 portable |
| `pico` | pico-sdk 2.3.1 (+ submodules), pico-examples sdk-2.3.1, picotool 2.3.1, RISC-V toolchain 16 |
| `stm32` | STM32Cube F4/F1/G4/H7 (sparse: Drivers + USB device library) |
| `wch` | ch32fun, openwch ch32v20x/ch32v307 EVT, wlink 0.1.3, wchisp 0.3.0 |
| `flash` | xPack OpenOCD 0.12.0-7, probe-rs 0.32.0 |
| `sdks` | esp-idf v6.1 (+ its `install.sh`, which checks its tools' sha256), py32f0-template, Kendryte SDKs + toolchain, bouffalo_sdk (+ legacy), arduino-cli 1.5.1 |
| `ide` | GoogleTest 1.18.0 for the CLion helper |
| `bend` | optional: Bend 2.0.39 (linux-x64) for the host-side `bend-host-check` example |

Not in the pins (by hand, see `toolchain/<sdk>/README.md`): k230 (`repo sync` of the frozen manifest in
`toolchain/k230`, ~8 GB), the Arduino cores (`arduino-cli core install`), the Bouffalo Xuantie toolchain, the host
private libs (`toolchain/host/README.md`), stlink/st-flash (built from source), the pyocd venv.
`mcu setup links` creates the symlinks into the sibling IDs (`tool/freertos`, `tool/zephyr`, `examples/{freertos,zephyr}`,
two `projects/`) when those IDs exist next to this one; they are not in git.

## bare SDK, Rust, Zig, Bend

`mcu build bare <chip> <example>`, `templates/tools/check.sh bare` (build + emulator run of every example/template).
Chips: `mps2-an385` (qemu), `virt-rv32` (qemu), `stm32f411`, `stm32h743`, `stm32f103`, `nrf52840`, `ch32v003` (Renode;
CH32V003 with a hand-written platform in `chips/ch32v003/renode.repl`). Every program prints `RESULT: PASS|FAIL`;
`toolchain/bare/run/firmware.sh <elf>` runs it in the chip's emulator. Projects may add `test.sh <target> <outdir>`
(size/symbol/UF2 checks, run by check.sh for any SDK) and `needs.sh` (exit 3 = SKIP when another ID is missing).

- **Rust: supported** (`rust-ffi` example, `bare/rust-app` template): no_std staticlib built with the rustc of
  `32_17_embedded_rust` (`$BOX_EMBEDDED_RUST`, else the sibling folder; reached, not copied), linked with the C startup.
- **Zig: supported** (`zig-ffi`, `bare/zig-app`): Zig 0.17.0 of `32_24_microzig` (`$BOX_MICROZIG`).
- **Bend 2: host side only** (`bend-host-check` example): Bend has no MCU target (its C output needs a hosted POSIX
  system), so it serves as a host reference for firmware logic, compared with the C firmware run in qemu. Bend is
  pinned here as an optional group (Bend 2.0.39, `mcu setup install bend`, sha256 checked) and runs with HOME/TMPDIR
  in `cache/bend-home`, BEND_NO_TELEMETRY=1; without it the example is skipped (exit 3). PASS 2026-10-29.

## Native mode

`mcu` needs no installation and has no native file (`box native enable` does not apply). It is reachable from anywhere
in two ways:

- **Box shell integration** (recommended): `box shell install --apply` adds one `# >>> box >>>` block to your rc files
  (bash, zsh, fish). It puts the generated PATH shim `mcu` (`box_env.d/bin/mcu`, a one-line `exec` into `tool/mcu`) and
  command-name completion on PATH. `box shell status` shows it, `box shell uninstall --apply` removes it.
- **By hand / standalone** (no Box top layer): mcu never edits your rc files; add this yourself.

```sh
export PATH="$BOX/30_39_tools/32_embedded/32_01_mcu/tool:$PATH"
fpath=($BOX/30_39_tools/32_embedded/32_01_mcu/tool/completion $fpath)   # zsh, before compinit
autoload -Uz compinit && compinit && compdef _mcu mcu
source $BOX/30_39_tools/32_embedded/32_01_mcu/tool/completion/mcu.bash  # bash
```

The completion in `tool/completion/` is richer (SDKs, targets, examples) than the generated one (command names only).

## Tests

```sh
cd tool && ./mcu test              # bats suite (tests/*.bats); same as `box test 32_01` from anywhere
./mcu check-plugins                # plugin contract
templates/tools/check.sh <sdk>     # build every example and template of an SDK
box scripts check 32_01            # the [scripts.mcu] descriptor matches `mcu help`
```

Last known (NOTES.md, 2026-10-29): bats 59/59; check.sh bare pico stm32 wch freertos: 350 builds/runs, 0 failed
(bare 13 examples + 12 templates, every one run in qemu/Renode).
Before (2026-10-25): bats 57/57; `check.sh` all SDKs 0 failed (avr 49, bouffalo 39, freertos 71, k210 24, k230 28,
pico 108, py32 38, stm32 48, wch 43; esp-idf final run was pending at hand-off); `mcu doctor` clean for all SDKs
(only optional dfu-util warns). Examples are "builds; untested on hardware".

## Troubleshooting

| Symptom | Fix |
|---|---|
| `unknown sdk` from `new` / `check.sh` | `MCU_ROOT` must be `32_01_mcu/tool`, not `32_01_mcu` (fixed 2026-10-24); run from `tool/` |
| `IDF_PATH is changed` (esp-idf) | enter through `tool/esp-idf` is fine now (env.sh uses `pwd -P`); do not source IDF's `export.sh` yourself |
| `mcu doctor` warns about dfu-util | optional; only needed for DFU flashing |
| Flash cannot find the probe / permission denied | install udev rules (needs sudo): `sudo cp toolchain/tools/udev/*.rules /etc/udev/rules.d/ && sudo udevadm control --reload && sudo udevadm trigger` |
| Old `Box/Tools/...` path strings in binaries | cosmetic (DWARF / `__FILE__`) in patchelf, libusb, stlink, picotool |
| Update broke an SDK | `./mcu rollback <sdk>` |

## Limitations / needs you

- Nothing was run on hardware (no board attached): flash, monitor and the examples are build-checked only.
- udev rules need `sudo` once (see above).
- `vendor_apps/jbdtools` is a Windows exe (Wine needed).
- Open items: `$BOX/00_09_meta/02_logs/2026_10_25_open_items.md`.

## Scripts

Every script of the tool prints usage with `-h`/`--help` (exit 0 ok, 1 failure, 2 usage error, 3 not implemented). The full list
(dispatcher, `lib/*.sh`, tests, completion, template tools) is in the `## Scripts` section of [tool/README.md](tool/README.md#scripts).
The launcher is described for the Box top layer in `.box.toml` (`[tool]`, `[scripts.mcu]`); `box describe mcu` shows it.

## Links

- [tool/README.md](tool/README.md) (command reference, layout), [tool/PLUGIN.md](tool/PLUGIN.md) (plugin contract), [tool/BOARDS.md](tool/BOARDS.md)
- [EXAMPLES_MATRIX.md](EXAMPLES_MATRIX.md), `examples/README.md`, `templates/README.md`, `templates/SPEC.md`
- `ide/clion/README.md`, `projects/`, `vendor_apps/`
- [NOTES.md](NOTES.md)
- Reports in `$BOX/00_09_meta/02_logs/`: `2026_10_24_mcu_examples_report.md`, `2026_10_24_mcu_plugins_{A,B,C}_report.md`, `2026_10_25_readability_embedded_core_report.md`
