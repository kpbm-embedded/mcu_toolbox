# 32_01_mcu notes

## 2026_10_24
- Reorg wrap-up (report: 00_09_meta/02_logs/2026_10_24_embedded_wrapup_report.md). `mcu doctor` run for all 13 SDKs.
- Repaired ELF interpreters that still pointed to `Box/Tools/Embedded/...` or `20_29_embedded/21_mcu/...`: `toolchain/host/patchelf-0.18.0/patchelf`, `toolchain/wch/{bin,ch32fun/minichlink}/minichlink`, 108 binaries in `toolchain/k230/host/` (bison, rsync, m4, flex, mtools, ...). Each now uses `toolchain/host/buildtools` ld-linux, patched on a new inode.
- Rewrote the stale prefixes in the `_sysconfigdata` files of `toolchain/{host,tools,k230/host}/python/*` using the move log.
- `toolchain/stm32/sdk.sh`: the "bundled toolchain" check compares resolved paths. It used `*/mcu/tools/toolchains/*`, which no longer matched, so it warned falsely.
- `<build disk>/build/mcu` changed to `$MCU_OUT_BASE` in 146 files (env.sh of pico/k210/k230, Makefiles, templates/tools, CLion `.idea`/`.run`, READMEs). `tool/PROGRESS_embedded.md` (history) and vendor `.map` files are unchanged.
- `ide/clion` (mcu-ide) was broken by the move. Fixed: `MCU_ROOT` is now `32_01_mcu/tool`, the python path in `mcu-ide`, the CLion default `~/.local/share/JetBrains/Toolbox/apps/clion-3`, `../../ide/clion/mcu-ide` in generated run configs, and the `MCU_ROOT` / `_MCU_ROOT` defaults in `cmake/*.cmake`, the templates and all real `projects/*` (CMakeLists, tests, cmake/toolchain.cmake, app/Makefile).
- Docs: `tool/README.md` has a new "Where this lives" section, plus fixes in `tool/PLUGIN.md` (MCU_ROOT, physical-path rule), `tool/BOARDS.md`, `tool/.shellcheckrc` and `toolchain/k230/README.md` (re-fetch steps).
- k230: the `sdk`/`toolchain` symlinks still point to the deleted `<build disk>/build/mcu/k230` and need a re-fetch (not done: ~8 GB download).
- k230 re-fetch (evening; report: 00_09_meta/02_logs/2026_10_24_k230_refetch_report.md): `toolchain/k230/{sdk,toolchain}` now point to `$MCU_OUT_BASE/k230/{sdk,toolchain}`. `repo sync` was pinned to `manifest-frozen-20261023.xml` (sdk effb2f82e822, 4.9 GB, 147 s). `make dl_toolchain` fetched musl GCC 12.0.1 and Xuantie GCC 10.2.0 (3.1 GB, sha256 match README, 131 s). Fixed `k230/sdk.sh` doctor, which falsely flagged the buildtools as OS because of physical vs logical `MCU_ROOT`. Fixed `k230/env.sh`, which now exports `BISON_PKGDATADIR` and `M4` because bison had the old `Box/Tools/Embedded` prefix compiled in. `mcu doctor k230`: rc 0. `mcu build k230 k230d_rtos_bpi_zero hello`: OK, 3m40s. `mcu selfcheck k230`: OK. bats: 29 of 29 passed.
- 2026_10_24 plugins k210/bouffalo/wch/py32 post-reorg check (report: 00_09_meta/02_logs/2026_10_24_mcu_plugins_A_report.md): no functional breakage, no edits needed. Old-path strings remain only in DWARF/`__FILE__` data (`wch` minichlink debug info, `k210/out/*.bin` from 10-23). doctor, libcheck, check-plugins, clean selfcheck and real builds (k210 standalone, bouffalo bl616+bl808, wch ch32v003, py32 fresh EXTI_Toggle): all rc 0, output under idisk03/.../32_01_mcu/build.
- 2026_10_24 plugins esp-idf/pico/avr/stm32 post-reorg check (report: 00_09_meta/02_logs/2026_10_24_mcu_plugins_B_report.md): one fix. `esp-idf/env.sh` now uses `pwd -P`: IDF's export.sh resolves IDF_PATH physically and warned "IDF_PATH is changed" when entered through the `tool/esp-idf` symlink, which left IDF_TOOLS_PATH and the python env on the logical path. Now all three are physical and match `tools/idf-env.json`. No other edits. Old-path strings remain only in pico `picotool` (.rodata data-dir fallback; the exe dir is searched first and holds the data files) and DWARF in `picotool`/`libusb-1.0.a`. doctor for all 4: rc 0 (stm32 only warns optional dfu-util). Clean selfchecks: pico 7 s, avr <1 s, stm32 1 s, esp-idf esp32c3 blink 14 s. All output under idisk03/.../32_01_mcu/build.
- 2026_10_24 shared layers host/tools post-reorg check (report: 00_09_meta/02_logs/2026_10_24_mcu_plugins_C_report.md). Text files: no old paths. Fixes: (1) `host/buildtools/.../sbin/ldconfig` is static, so boxelfinterp skipped it; its `.sysdirs`/`.ldsocache`/`.gccrelocprefix` still named `Box/Tools/Embedded/mcu/host`. Each tree's copy was rewritten to its own sysroot (new inode). (2) 608 stale `.pyc` (co_filename only) in `tools/python` and `tools/venv` recompiled with the tree's own python. (3) 7 stale uv `interpreter-v4` cache entries in `host/.uv-cache` moved to Trash. All tools run from /tmp through env.sh; `mcu doctor` stm32/freertos/zephyr clean, `mcu test` 51/51. Left as is (cosmetic): DWARF/`__FILE__` old paths in patchelf, libusb, stlink, plus libudev's optional `udev.conf` path.
- 2026_10_24 embedded extras (00_09_meta/02_logs/2026_10_24_embedded_extras_report.md): rebuilt static libusb 1.0.30 (`--with-pic --disable-udev`, `-ffile-prefix-map`, OS gcc) for `tools/static` and `pico/libusb`, stlink v1.9.0 (`tools/stlink` bin + libstlink.so.1.9.0) and picotool v2.3.1 (sdk.sh cmake line + `-static-libstdc++ -static-libgcc`; ldd libc/libm only). Old binaries in Trash (logged). New binaries: 0 `/Box/Tools/` strings, no absolute source paths. Recipe: 00_09_meta/02_logs/2026_10_24_embedded_extras_artifacts/.
- 2026_10_24 examples/templates audit (report: 00_09_meta/02_logs/2026_10_24_mcu_examples_report.md, matrix: EXAMPLES_MATRIX.md). Fixed `templates/tools/check.sh` and `instantiate.py`: after the reorg, MCU_ROOT pointed at 32_01_mcu instead of 32_01_mcu/tool, so every SDK failed with "unknown sdk". check.sh also takes `MCU_EXAMPLES_OUT` now. `mcu new <sdk> <name> [target]` now instantiates templates/<sdk>/{minimal,minimal-*,*-minimal} (the one matching the target) into projects/<name>; the plugin's sdk_new is only the fallback. 5 new bats tests, 56/56 pass. Examples added or extended for every SDK except zephyr, which is owned by 32_03 and only build-checked read-only (80/80). check.sh: avr 49, bouffalo 39, freertos 71, k210 24, k230 28, pico 108, py32 38, stm32 48, wch 43, all failed 0; esp-idf final run was pending at hand-off. The k230 templates' board.h now uses the per-board pins. Backups: 00_09_meta/02_logs/config_backups/2026_10_24_223825_mcu_examples/.
- 2026_10_25 readability pass (report: 00_09_meta/02_logs/2026_10_25_readability_embedded_core_report.md): MCU_OUT_BASE/check.sh/completion defaults via $BOX_V3/$BOX_MCU; templates/tools: check.sh --help + first_file helper (no ls parsing), adapters shellcheck-clean (bouffalo reuses avr), Python tools with main()/argparse help. bats 56/56 before and after.
- 2026_10_25 readability pass 2 (report: 00_09_meta/02_logs/2026_10_25_readability2_embedded_core_report.md; backups: config_backups/2026_10_25_readability2_embedded_core/): new read-only `mcu where`; reserved commands exit 3 (not implemented) instead of 1; `mcu help` lists environment variables, exit status and examples; removed the unused `run_cmd` from lib/core.sh; sdks.bats: two `cmd | grep -q` (flaky under bats' pipefail, silently skipped SDKs) now use here-strings; cli.bats +1 test (where). bats 56/56 -> 57/57. README/tool/README: `box shell install` + shim `mcu` instead of sourcing box_env.sh, `where`, exit 3, $BOX paths.

## 2026-10-29
- Prepared for GitHub as `mcu` (git repo in this folder). `.gitignore` keeps sources only: plugins, env.sh, READMEs,
  board/chip layers, scripts and udev rules of `toolchain/<sdk>`; not the SDK trees, toolchains, emulators,
  `vendor_apps/`, `cache/`, bats, CLion's GoogleTest/workspace, build listings (`*.lst` left in examples/avr). The
  symlinks into 32_04_freertos / 32_03_zephyr are not committed: `mcu setup links` recreates them when the sibling IDs
  exist. Host paths removed from tracked files: pico/k210/k230 env and scripts use `${MCU_OUT_BASE:-<ID>/build}`, the
  avr config gets its directories and the local index URL from env.sh (`ARDUINO_DIRECTORIES_*`), project run
  configurations use `$PROJECT_DIR$/../../toolchain`, docs say `$BOX/<ID path>` / `$MCU_OUT_BASE`; `.idea/cmake.xml` of the
  projects is not committed (one machine's paths).
- New `mcu setup` (tool/setup.sh, config/pins.tsv): 34 pins in groups core, bare, pico, stm32, wch, flash, sdks, ide;
  archives sha256-checked (publisher checksum files where they exist; the release archives of wlink, wchisp, picotool,
  pico RISC-V toolchain, arduino-cli and the Kendryte toolchain were compared with what is installed here), git pins by
  commit, sparse Cube clones, `post=` (esp-idf `install.sh`). Not pinned: k230 repo tree, Arduino cores, Bouffalo
  toolchain, host private libs, stlink, pyocd venv (README "Setup").
- New SDK plugin `bare` (toolchain/bare, contract v2): shared CMake toolchain files (arm-none-eabi, riscv-none-elf),
  `bare_chip.cmake`/`bare.cmake`, 7 chip layers (mps2-an385, virt-rv32, stm32f411, stm32h743, stm32f103, nrf52840,
  ch32v003) with startup, linker scripts and console, `run/firmware.sh` (qemu via semihosting/SiFive test device exit;
  Renode headless with a UART file backend for a fixed emulated time). Renode: stock stm32f412/h743/f103 models,
  nRF52840 with the legacy (non-EasyDMA) UART, a hand-written CH32V003 platform. `langs.cmake`: Rust (rustc of
  32_17_embedded_rust) and Zig (32_24_microzig) reached via `$BOX_EMBEDDED_RUST` / `$BOX_MICROZIG` or the sibling folder.
- 13 examples (examples/bare) and 12 templates (templates/bare), listed in README. check.sh: `test.sh <target> <outdir>`
  of any project runs after its build; `needs.sh` exit 3 = SKIP (counted separately). Problems found: Renode's stock
  nRF52840 UART is EasyDMA-only (own repl), newlib crt0 clashes on RISC-V (`-nostartfiles`, constructors walked in
  rv_start.S), Zig objects need `__exidx_start/end`, emulators do not keep TIM2 and SysTick in ratio (test changed to
  "no interrupt after CEN=0"), F1 LL pin-mode read-back differs (ODR read-back instead), an IT-block typo.
- bats: +2 tests (setup list/usage, every pin has sha256 or commit) -> 59/59. Rust and Zig supported (examples rust-ffi,
  zig-ffi; templates rust-app, zig-app). Correction (third language = Bend 2, not BEAM; no BEAM was added): example
  bend-host-check (host Bend reference vs. the C firmware in qemu), skipped (exit 3) because 32_31_bend does not exist
  yet; its C part was checked by hand in qemu (PASS), the Bend file is unchecked. No MCU claim for Bend.
- bats test "targets and examples work for every buildable sdk" skips SDKs whose doctor fails (not installed): it
  failed in the fresh clone (avr not installed). Fresh clone: see below.
- Results: check.sh bare 64 (33 example runs on 7 chips + 31 template runs, all in qemu/Renode, incl. Rust/Zig), pico 108,
  stm32 48, wch 43, freertos 87 (now incl. the 16 qemu examples of 32_04): 350 OK, 0 failed; bats 59/59; mcu lint 0
  findings in owned files; doctor bare ok. Nothing ran on hardware (flash commands of the bare plugin are untested).
- Fresh-clone check (clone on idisk03, `cache/` linked to this one, `BOX_INTEGRATION=0`, output inside the clone,
  Rust/Zig reached via `BOX_EMBEDDED_RUST`/`BOX_MICROZIG`): all 34 pins missing at first; `mcu setup install core bare
  pico` reinstalled buildtools, cmake, ninja, Arm GNU, xPack RISC-V GCC, qemu Arm/RISC-V, Renode, pico-sdk (+ submodules,
  from GitHub), pico-examples, picotool, pico RISC-V toolchain in 1 min 48 s (archives from the cache); `mcu devtools
  install` (sha256 ok); `mcu setup links` skipped (no sibling IDs next to the clone); doctor bare/pico ok; check.sh bare
  65: 61 OK, 4 SKIP (bend-host-check: no 32_31_bend; stm32-ll x3: Cube packs not installed in the clone); `mcu build
  pico pico blink` and the pico minimal template on 6 targets OK; bats 59/59 (after making the SDK test skip uninstalled
  SDKs; the first clone run failed it on avr). Not reinstalled in the clone: stm32, wch, flash, sdks, ide groups and
  everything unpinned (k230, Arduino cores, Bouffalo toolchain, host libs, stlink, pyocd). Clones moved to the Trash.
