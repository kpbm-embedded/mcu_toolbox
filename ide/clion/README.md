# mcu/clion: CLion integration

CLion 2026.2 (build 262.10968.176) is supported. Everything here is **added**: no SDK tree, no
existing workspace file and nothing under `~/.config/JetBrains` is changed. Each project keeps all
its CLion settings in its own `.idea/` (CMake profiles) and `.run/` (run configurations).

```
mcu/clion/
  mcu-ide            CLI: new | workspace | list | build | test | flash | monitor | gdbserver | verify
  new-project        = mcu-ide new <sdk> <name> [target] [example]
  lib/mcu_ide.py     generator + runner (host CPython 3.12, -I, stdlib only)
  lib/mcu_index.py   make dry run / compile_commands.json -> CMake "firmware-index" target
  cmake/             McuProject.cmake (make-driven firmware), McuHostTests.cmake (GoogleTest + CTest)
  templates/common/  lib/ringbuf + tests/ copied into every project
  third_party/       googletest-1.18.0 (dl/: release tarball + .sha256; matches GitHub's asset digest)
  workspace/         small CLion project that only holds run configurations for ./mcu
mcu/projects/<name>  your projects (one CLion project each)
```

## 1. The workspace project
File > Open > `mcu/clion/workspace`. It has no CMake and indexes only its own folder. The run
widget groups the Shell Script configs into folders:
- **Workspace**: `mcu doctor all`, `selfcheck all`, `libcheck all`, `list`, `update all` (dry run,
  `MCU_YES=0`), `list CLion projects`.
- **one folder per SDK**: build / flash / monitor of the SDK's default target, which is its
  `SDK_SELFCHECK` (e.g. `zephyr: build blackpill_f411ce samples/basic/blinky`). Edit the script text to
  add a port or probe (`../../mcu flash stm32 blackpill-f411 blinky-f411 openocd`).

Regenerate after adding an SDK: `mcu/clion/mcu-ide workspace` (the old `.run/` goes to the Trash).

## 2. Open a project
File > Open > `mcu/projects/<name>`, and trust the project. CLion loads three CMake profiles from
`.idea/cmake.xml`:

| Profile | Purpose | Build dir |
|---|---|---|
| Debug | firmware (Zephyr: `CONFIG_DEBUG_OPTIMIZATIONS=y`; py32: `OPT=-Og`) | `$MCU_OUT_BASE/projects/<name>/cmake-build-debug` |
| Release | firmware with the SDK's default optimisation | `.../cmake-build-release` |
| Host-Tests | `tests/` on the PC with buildtools gcc 15.3 + GoogleTest, run by CTest | `.../cmake-build-host-tests` |

Build (Ctrl+F9) compiles the firmware natively. The Debug profile's artifacts are what Flash and
Debug use. Every project's README lists its run configurations.

## 3. Create a project
```
mcu/clion/new-project <sdk> <name> [target] [example]      # or: mcu/clion/mcu-ide new ...
mcu/clion/new-project zephyr my-app weact_g431 samples/basic/button
mcu/clion/new-project pico pico-usb pico hello_usb
```
The SDK example is **copied** into `mcu/projects/<name>`. Then `lib/ringbuf` and `tests/` are added,
followed by `.idea/cmake.xml`, `.idea/misc.xml`, `.run/*.run.xml`, `mcu-project.json` and `README.md`.
`--force` moves an existing project to `~/.local/share/Trash/files` and recreates it.

**Pluggable code source (`--source DIR`)**: every SDK spec resolves its example dir through one hook,
`example_src()` in `lib/mcu_ide.py`. `new <sdk> <name> [target] --source DIR` copies DIR instead of the
SDK example. DIR can be an instantiated `mcu/templates/<sdk>/<t>` or an `mcu/examples/<sdk>/<e>` dir with
the same layout the SDK example has. The CLion layer is added on top unchanged (`.idea`, `.run`,
Host-Tests, index).

| sdk | targets (default first) | how CLion builds it |
|---|---|---|
| zephyr | blackpill_f411ce, any `mcu targets zephyr` name or west board | CMake: `find_package(Zephyr)`, `-DBOARD` |
| stm32, freertos | blackpill-f411 | CMake + `cmake/arm-none-eabi.cmake` toolchain file, `-DCUBE`, `-DFREERTOS_KERNEL` |
| pico | pico2, pico, zero, rp2350-zero | CMake: `pico_sdk_import.cmake`, `-DPICO_BOARD/PLATFORM/SDK_PATH/TOOLCHAIN_PATH` |
| esp-idf | esp32s3, esp32, esp32c3, ... | CMake: IDF `project.cmake`, `-DIDF_TARGET`, sdkconfig in the build dir, IDF env |
| k210 | cm1 (flash alias for standalone) | the standalone SDK's own CMake modules, used out of tree (`-DSDK_ROOT -DTOOLCHAIN`) |
| wch | ch32v003, ch32v203, ch32v307 | thin CMake -> ch32fun's `ch32fun.mk` (make) |
| py32 | py32f003x8, py32f002ax5, py32f030x8 | thin CMake -> py32f0-template Makefile (make, symlinked stage dir) |
| bouffalo | bl616, bl702 | thin CMake -> bouffalo_sdk `project.build` (make + its inner CMake) |
| avr | nano, nano-old, mega, attiny85, any FQBN | thin CMake -> `arduino-cli compile` |
| k230 | k230d_rtos_bpi_zero, k230_rtos_01studio | thin CMake -> copy of the SDK's helloworld Makefile including the SDK `.mk` files |

### Why some SDKs use a thin CMake around make
ch32fun, the py32 template, bouffalo's `project.build`, arduino-cli and the K230 app makefiles are not
plain CMake. A CLion *Makefile project* would need the make executable and environment from the
IDE's global toolchain, and that toolchain can't be changed here. So these projects are CMake
projects with two targets:
- `firmware` (ALL): runs the SDK's own make with the SDK's flags. This is what Build runs.
- `firmware-index` (OBJECT, `EXCLUDE_FROM_ALL`, never built): `lib/mcu_index.py` fills it at
  configure time. It takes the SDK's real per-file flags from `make -n -B -w`, or from the
  compile_commands.json that arduino-cli (`--only-compilation-database`) or bouffalo's inner CMake
  writes. CLion therefore indexes the project and SDK sources with the exact `-D/-I/-m` flags. The
  `.ino` sketch is indexed through a wrapper TU that includes it after `Arduino.h`.

Make SDKs that write into their own tree are kept out of it:
- ch32fun regenerates its linker script on every build. The project Makefile uses
  `override GENERATED_LD_FILE` to keep it in the build dir.
- The py32 template runs in a stage dir of symlinks under the build dir.
- bouffalo gets a `BUILD_DIR` that points into the build dir.
- K230 objects and the ELF go to `OUT`.

## 4. How CLion maps to the mcu tools
- **CMake**: CLion's bundled CMake 4.3.1 from the "Default" toolchain. The generator (`-G Ninja`) and
  `-DCMAKE_MAKE_PROGRAM=mcu/host/bin/ninja` are set in the profile, and the compilers come from the
  toolchain file or the SDK. The environment block in `.idea/cmake.xml` is `<sdk>/env.sh`, captured in
  a clean shell. Its `PATH` is the SDK's tools plus `mcu/host/sysbin` (the strict PATH of
  `mcu selfcheck`), and it also carries the SDK variables (`ZEPHYR_BASE`, `IDF_PATH`, `IDF_TOOLS_PATH`,
  `PICO_SDK_PATH`, `BL_SDK_BASE`, `ARDUINO_CONFIG_FILE`, `SDK_TOOLCHAIN_DIR`, ...). `PYTHONPATH` and
  `PYTHONHOME` are blanked. CLion applies the same environment to configure and build. The host cmake
  4.4.4 also works: `mcu-ide build <p> Debug --cmake mcu/host/cmake/bin/cmake`.
- **Re-capturing the env** after `mcu update` changes tool paths: `mcu-ide new <sdk> <name> ... --force`
  regenerates. Copy your sources back from the Trash, or diff the two `.idea/cmake.xml` files.
- **Flash**: Shell Script config, `mcu-ide flash .`. It builds the Debug profile, then points the
  dispatcher at this project's output with `MCU_OUT_BASE=<build>/.mcu-out` and runs
  `mcu flash <sdk> <target> <example>`. `.mcu-out/<sdk>/<target>/<example>` is a symlink into
  the build dir, laid out the way the plugin expects. The exceptions:
  - esp-idf: `mcu run esp-idf -- python -m esptool ... write-flash @flash_args`, because the plugin
    hard-codes `$IDF_PATH/examples`.
  - bouffalo: `mcu run bouffalo -- make flash` (BLFlashCommand).
  - k230: prints how to copy the RT-Smart ELF to the SD card. An app is not flashed.
  Extra args are passed through, e.g. a port or probe: `mcu-ide flash . openocd`.
- **Monitor**: Shell Script config run in a terminal, `mcu monitor <sdk> [port] [baud]`. For esp-idf it
  is `esp_idf_monitor` on the project ELF.
- **Debug**: run configuration type *Embedded GDB Server*
  (`com.jetbrains.cidr.embedded.customgdbserver.type`). It uses CLion's **bundled** multiarch GDB 17.1
  (ARM + RISC-V) via `<debugger kind="GDB" isBundled="true"/>`, and builds the CMake target first.
  The server executable is stored in the config itself. The *OpenOCD Download & Run* type is not
  used because its OpenOCD path is a global IDE setting.

  | Project | GDB server |
  |---|---|
  | zephyr-blinky-blackpill, stm32-blinky-f411, freertos-blinky-f411 | `tools/xpack-openocd` + ST-Link (stm32f4x) |
  | pico2-blink | xpack-openocd + CMSIS-DAP debugprobe (rp2350) |
  | py32f003-toggle | `py32/.venv/bin/pyocd gdbserver` with the Puya pack |
  | ch32v003-blink | `wch/bin/minichlink -baG` (WCH-LinkE) |
  | bl616-hello | bouffalo_sdk's OpenOCD + FT2232 (Sipeed RV-Debugger Plus), `tgt_e907.cfg` |

  `mcu-ide gdbserver <p>` starts the same server from a shell.
  The schemas were taken from the CLion 2026.2 jars:
  - type and factory ids from `CustomGdbServerConfigurationType` and `ShConfigurationType`;
  - attributes from `GdbServerRunConfiguration` and `CustomGdbServerRunConfiguration`
    (read/writeExternal), `DebuggerData` (`<debugger kind isBundled>`), `ShRunConfiguration`
    (`SCRIPT_TEXT`...), `CMakeSettingsStorageKt` (`cmake.xml`) and `CTestAppRunConfigurationType`.
- **Tests**: select the Host-Tests profile and run *Host tests* (CTest) or the gutter icons next to
  each `TEST()`. To add a test, put target-independent code in `lib/`, add a library and a
  `mcu_add_host_test()` line in `tests/CMakeLists.txt`. Command line: `mcu-ide test <p>`.

## 5. Limits
- **No Debug config**:
  - esp32s3: Xtensa. The bundled GDB has no Xtensa. Use `mcu shell esp-idf`, then `idf.py openocd` +
    `xtensa-esp32s3-elf-gdb`.
  - k230: an RT-Smart user process; the SDK image has no on-target gdbserver.
  - k210: no K210-capable OpenOCD in the workspace.
  - avr: no debugWIRE server.
- **CMake versions**: CLion's bundled CMake (4.3.1) differs from `mcu/host` (4.4.4). Both work for
  every project. Don't mix them on one build dir; `mcu-ide` always uses the cmake recorded in the cache.
- **Absolute paths**: `cmake.xml` and the Debug configs hold absolute paths into this workspace.
  Regenerate (`--force`) if the workspace moves.
- **Build dirs** are on `$MCU_OUT_BASE/projects` (`MCU_IDE_BUILD_ROOT` overrides this when
  generating).
- **esp-idf**: the blink example pulls `led_strip` through the IDF component manager on the first
  configure, which needs network. It lands in the project's `managed_components/`.
- **SDK tools that write into their own tree**: `make flash` for bouffalo (BLFlashCommand) creates
  `log/`, `img_create/` and `eflash_loader_cfg.ini` under `bouffalo_sdk/tools/bflb_tools/bouffalo_flash_cube`.
  These are git-ignored, and `mcu flash bouffalo` does the same. arduino-cli bumps a counter in
  `avr/data/inventory.yaml` on every compile. `mcu-ide flash|monitor|gdbserver` set
  `PYTHONDONTWRITEBYTECODE=1`, so west and esptool leave no `__pycache__` in the SDK trees.
- **py32**: the template's `rules.mk` calls `expr`, which is not in the strict PATH. The build passes
  `IS_GCC_ABOVE_12=1`, so the harmless "expr: command not found" line has no effect. `mcu monitor py32`
  needs pyserial in `py32/.venv`, which is missing (an SDK-side issue; this layer leaves the venv alone).
  pyOCD waits for a probe until one is plugged in.
- **Embedded GDB Server / CTest XML**: these were written from the jar schemas. CLion may rewrite them
  in its own attribute order on first save; that is expected.

## 6. Proof that nothing else changed
Before any file was added:
- `git rev-parse HEAD` + `git status --porcelain` were recorded for all 265 repos under `mcu/` (west
  and IDF submodules included) plus the 13 `repo` projects of `k230/sdk` (a symlink to <build disk>).
- A sha256 manifest of the 116k non-repo files was taken, excluding clion/, projects/, .git, caches and
  build dirs, as were hashes of `~/.config/JetBrains/CLion2026.2/options` and `k230/sdk/.config`.

The same snapshot is compared afterwards. The integrity scripts and results are in the session
scratchpad, and the hand-off report summarises them.

`mcu ide` hook for the dispatcher (one line in the `case "$cmd"` block):
```
  ide) exec "$MCU_ROOT/clion/mcu-ide" "$@" ;;
```
