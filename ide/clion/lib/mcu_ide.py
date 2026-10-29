"""mcu-ide: CLion integration for the mcu workspace (generator + command-line runner).

  mcu-ide new <sdk> <name> [target] [example] [--force]   create mcu/projects/<name> (copies the SDK example)
  mcu-ide workspace                                      (re)write mcu/clion/workspace/.run/*.run.xml
  mcu-ide list                                           projects and what they build
  mcu-ide build <project> [profile] [--cmake PATH]       configure + build exactly like CLion (reads .idea/cmake.xml)
  mcu-ide test <project> [--cmake PATH]                  build profile Host-Tests and run ctest
  mcu-ide flash <project> [port|probe] [--profile P] [--no-build]
  mcu-ide monitor <project> [port] [baud]
  mcu-ide gdbserver <project>                            start the debug server used by the "Debug" run config
  mcu-ide verify <project...|all>                        headless check: build, tests, XML, dry runs (no device writes)

<project> is a name under mcu/projects or a path. Nothing here edits an SDK tree or the IDE's global
config: everything CLion needs lives in the project's .idea/ and .run/ (see mcu/clion/README.md).
Run with the host python in isolated mode (the mcu-ide wrapper does that).
"""
import argparse
import json
import os
import re
import shlex
import shutil
import subprocess
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

MCU_ROOT = Path(__file__).resolve().parents[3] / "tool"   # 32_01_mcu/tool: has clion/ host/ tools/ projects/ (symlinks into the ID)
CLION_DIR = MCU_ROOT / "clion"
PROJECTS = MCU_ROOT / "projects"
# the mcu build root: MCU_OUT_BASE (mcu env), else this ID under the Box `out` location ($BOX_OUT, old $BOX_V3),
# else <ID>/build; MCU_IDE_BUILD_ROOT overrides it all
_ID_DIR = Path(__file__).resolve().parents[3]
_OUT = os.environ.get("BOX_OUT") or os.environ.get("BOX_V3")
_MCU_OUT = os.environ.get("MCU_OUT_BASE") or (str(Path(_OUT) / "30_39_tools/32_embedded/32_01_mcu/build") if _OUT
                                               else str(_ID_DIR / "build"))
BUILD_ROOT = Path(os.environ.get("MCU_IDE_BUILD_ROOT", str(Path(_MCU_OUT) / "projects")))
CLION_HOME = Path(os.environ.get("CLION_HOME", str(Path.home() / ".local/share/JetBrains/Toolbox/apps/clion-3")))
CLION_CMAKE = CLION_HOME / "bin/cmake/linux/x64/bin/cmake"
CLION_NINJA = CLION_HOME / "bin/ninja/linux/x64/ninja"
HOST_BT = MCU_ROOT / "host/buildtools/sysroots/x86_64-pokysdk-linux/usr/bin"
HOST_NINJA = MCU_ROOT / "host/bin/ninja"
SYSBIN = MCU_ROOT / "host/sysbin"
JOBS = int(os.environ.get("MCU_JOBS", "16"))
PROFILES = ("Debug", "Release", "Host-Tests")
OPENOCD = MCU_ROOT / "tools/xpack-openocd/bin/openocd"
OPENOCD_SCRIPTS = MCU_ROOT / "tools/xpack-openocd/openocd/scripts"
DROP_ENV = {"_", "SHLVL", "PWD", "OLDPWD", "PS1", "VIRTUAL_ENV_PROMPT", "IDF_DEACTIVATE_FILE_PATH",
            "IDF_PY_COMP_WORDBREAKS", "BB_ENV_PASSTHROUGH_ADDITIONS", "HOME", "USER", "LANG", "TERM"}


def die(msg):
    sys.stderr.write("mcu-ide: %s\n" % msg)
    sys.exit(1)


def say(msg):
    print(msg, flush=True)


# --------------------------------------------------------------------------------------------- env
def sdk_env(sdk):
    """Variables <sdk>/env.sh adds, sourced in a clean env whose PATH is the strict host/sysbin."""
    base = {"HOME": os.environ.get("HOME", "/tmp"), "USER": os.environ.get("USER", ""),
            "LANG": "C.UTF-8", "TERM": "dumb", "PATH": str(SYSBIN)}
    envsh = "host/env.sh" if sdk == "host" else "%s/env.sh" % sdk
    script = 'source "$1" >/dev/null 2>&1; env -0'
    out = subprocess.run(["/bin/bash", "-c", script, "_", str(MCU_ROOT / envsh)], env=base,
                         stdout=subprocess.PIPE, check=True, timeout=300).stdout.decode()
    env = dict(x.split("=", 1) for x in out.split("\0") if "=" in x)
    res = {k: v for k, v in env.items() if base.get(k) != v and k not in DROP_ENV}
    # make sure nothing inherited from the desktop session redirects the managed pythons
    res.setdefault("PYTHONPATH", "")
    res.setdefault("PYTHONHOME", "")
    return dict(sorted(res.items()))


# ------------------------------------------------------------------------------------- SDK helpers
def sdk_selfcheck(sdk):
    txt = (MCU_ROOT / sdk / "sdk.sh").read_text(errors="replace")
    m = re.search(r'^SDK_SELFCHECK="([^"]*)"', txt, re.M)
    return m.group(1).split() if m else []


def zephyr_board(target):
    txt = (MCU_ROOT / "zephyr/sdk.sh").read_text()
    for line in txt.splitlines():
        f = line.strip().strip("'").split("|")
        if len(f) >= 4 and f[0].lstrip("_ZTARGETS='") == target:
            return f[1]
    return target


def cmake_project_name(cmakelists):
    m = re.search(r"^\s*project\s*\(\s*([A-Za-z0-9_.+-]+)", Path(cmakelists).read_text(), re.M | re.I)
    return m.group(1) if m else None


SOURCE_OVERRIDE = None  # set by `new --source DIR`


def example_src(default):
    """Hook: where a new project's code comes from. Default is the SDK example dir; `new --source DIR`
    (e.g. an instantiated mcu/templates/<sdk>/<t> or an mcu/examples/<sdk>/<e> dir) replaces it. The
    directory must have the same shape as the SDK example the spec expects."""
    return Path(SOURCE_OVERRIDE).resolve() if SOURCE_OVERRIDE else Path(default)


def copytree(src, dst, ignore=()):
    shutil.copytree(src, dst, dirs_exist_ok=True, symlinks=False,
                    ignore=shutil.ignore_patterns("build", "build_*", ".git", *ignore))


HOST_BLOCK = """# >>> mcu/clion: host unit tests, CMake profile "Host-Tests" (see README.md)
if(MCU_HOST_TESTS)
  project({name}_host_tests C CXX)
  enable_testing()
  add_subdirectory(tests)
  return()
endif()
# <<< mcu/clion
"""


def add_host_block(cmakelists, name):
    """Insert the Host-Tests switch right after cmake_minimum_required() of a copied CMakeLists."""
    txt = Path(cmakelists).read_text()
    m = re.search(r"^\s*cmake_minimum_required\([^)]*\)\s*\n", txt, re.M | re.I)
    block = HOST_BLOCK.format(name=name)
    if m:
        txt = txt[:m.end()] + block + txt[m.end():]
    else:
        txt = "cmake_minimum_required(VERSION 3.20)\n" + block + txt
    Path(cmakelists).write_text(txt)


# ---------------------------------------------------------------------------- debug server presets
def openocd_debug(label, *cfgs, extra=()):
    args = ["-s", str(OPENOCD_SCRIPTS)]
    for c in cfgs:
        args += ["-f", c]
    args += list(extra) + ["-c", "gdb port 3333", "-c", "telnet port disabled", "-c", "tcl port disabled"]
    return {"label": label, "exe": str(OPENOCD), "args": args, "connect": "tcp:localhost:3333",
            "reset": "monitor reset halt", "check": ["-c", "shutdown"]}


STLINK_F4 = lambda: openocd_debug("OpenOCD + ST-Link", "interface/stlink.cfg", "target/stm32f4x.cfg")
ZEPHYR_DEBUG = {
    "blackpill_f411ce": ("interface/stlink.cfg", "target/stm32f4x.cfg"),
    "weact_f405": ("interface/stlink.cfg", "target/stm32f4x.cfg"),
    "weact_g431": ("interface/stlink.cfg", "target/stm32g4x.cfg"),
    "weact_g030": ("interface/stlink.cfg", "target/stm32g0x.cfg"),
    "weact_h743": ("interface/stlink.cfg", "target/stm32h7x.cfg"),
    "bluepill_f103": ("interface/stlink.cfg", "target/stm32f1x.cfg"),
    "rpi_pico": ("interface/cmsis-dap.cfg", "target/rp2040.cfg"),
    "rpi_pico2": ("interface/cmsis-dap.cfg", "target/rp2350.cfg"),
}


# ------------------------------------------------------------------------------ project specs
def spec_zephyr(name, target, example, d):
    target = target or "blackpill_f411ce"
    example = example or "samples/basic/blinky"
    zb = MCU_ROOT / "zephyr/ws/zephyr"
    src = example_src(Path(example) if example.startswith("/") else zb / example)
    if not (src / "CMakeLists.txt").exists():
        die("zephyr example not found: %s" % src)
    copytree(src, d, ignore=("sample.yaml", "pytest"))
    if (d / "README.rst").exists():
        (d / "README.rst").rename(d / "README.example.rst")
    proj = cmake_project_name(d / "CMakeLists.txt")
    add_host_block(d / "CMakeLists.txt", name)
    board = zephyr_board(target)
    py = MCU_ROOT / "zephyr/.venv/bin/python"
    dbg = None
    if target in ZEPHYR_DEBUG:
        iface, tgt = ZEPHYR_DEBUG[target]
        dbg = openocd_debug("OpenOCD + " + ("ST-Link" if "stlink" in iface else "CMSIS-DAP"), iface, tgt)
    return {
        "kind": "cmake", "env_sdk": "zephyr",
        "options": ["-DBOARD=%s" % board, "-DPython3_EXECUTABLE=%s" % py],
        "profile_options": {"Debug": ["-DCONFIG_DEBUG_OPTIMIZATIONS=y"], "Release": []},
        "cmake_project": proj, "target": "zephyr_final",
        "elf": "zephyr/zephyr.elf",
        "flash": {"how": "mcu", "sdk": "zephyr", "target": target, "example": "app", "stage": "{gen}"},
        "monitor": ["zephyr"], "debug": dbg,
        "debug_skip": None if dbg else "no probe preset for %s; add one in mcu-project.json" % target,
        "about": "Zephyr app (find_package(Zephyr)), board %s" % board,
    }


def _stm32_common(name, d, ex_src, cmakelists):
    copytree(ex_src / "src", d / "src")
    copytree(MCU_ROOT / "stm32/examples/common", d / "cmake")
    (d / "CMakeLists.txt").write_text(cmakelists)
    return {
        "kind": "cmake", "env_sdk": None,
        "toolchain": "cmake/arm-none-eabi.cmake",
        "cmake_project": name, "target": name, "elf": name + ".elf",
        "debug": STLINK_F4(), "debug_skip": None,
    }


def spec_stm32(name, target, example, d):
    target = target or "blackpill-f411"
    example = example or "blinky-f411"
    ex = example_src(MCU_ROOT / "stm32/examples" / example)
    cm = """cmake_minimum_required(VERSION 3.20)
{host}
# Firmware: STM32F411CEU6 + STM32CubeF4 HAL from mcu/stm32 (CUBE comes from the CMake profile).
if(NOT CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE ${{CMAKE_CURRENT_LIST_DIR}}/cmake/arm-none-eabi.cmake)
endif()
project({name} C ASM)
include(${{CMAKE_CURRENT_LIST_DIR}}/cmake/stm32f411.cmake)
add_executable({name} src/main.c)
target_link_libraries({name} stm32f411_hal)
stm32f411_finish({name})
""".format(name=name, host=HOST_BLOCK.format(name=name))
    s = _stm32_common(name, d, ex, cm)
    s.update({"env_sdk": "stm32", "options": ["-DCUBE=%s" % (MCU_ROOT / "stm32/STM32CubeF4")],
              "flash": {"how": "mcu", "sdk": "stm32", "target": target, "example": name, "stage": "{gen}"},
              "monitor": ["stm32"], "about": "STM32CubeF4 HAL blinky (example %s)" % example})
    return s


def spec_freertos(name, target, example, d):
    target = target or "blackpill-f411"
    example = example or "blinky-f411-freertos"
    ex = example_src(MCU_ROOT / "freertos/examples" / example)
    cm = """cmake_minimum_required(VERSION 3.20)
{host}
# Firmware: FreeRTOS-Kernel (mcu/freertos) + STM32CubeF4 HAL (mcu/stm32); paths come from the CMake profile.
if(NOT CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE ${{CMAKE_CURRENT_LIST_DIR}}/cmake/arm-none-eabi.cmake)
endif()
project({name} C ASM)
include(${{CMAKE_CURRENT_LIST_DIR}}/cmake/stm32f411.cmake)
set(FREERTOS_KERNEL "" CACHE PATH "FreeRTOS-Kernel checkout")
set(FRK ${{FREERTOS_KERNEL}})
add_library(freertos STATIC ${{FRK}}/tasks.c ${{FRK}}/list.c ${{FRK}}/queue.c ${{FRK}}/timers.c
  ${{FRK}}/portable/GCC/ARM_CM4F/port.c ${{FRK}}/portable/MemMang/heap_4.c)
target_include_directories(freertos PUBLIC ${{CMAKE_CURRENT_LIST_DIR}}/src ${{FRK}}/include ${{FRK}}/portable/GCC/ARM_CM4F)
target_link_libraries(freertos PUBLIC stm32f411_hal)
add_executable({name} src/main.c)
target_link_libraries({name} freertos)
stm32f411_finish({name})
""".format(name=name, host=HOST_BLOCK.format(name=name))
    s = _stm32_common(name, d, ex, cm)
    s.update({"env_sdk": "freertos",
              "options": ["-DCUBE=%s" % (MCU_ROOT / "stm32/STM32CubeF4"),
                          "-DFREERTOS_KERNEL=%s" % (MCU_ROOT / "freertos/FreeRTOS-Kernel")],
              "flash": {"how": "mcu", "sdk": "freertos", "target": target, "example": name, "stage": "{gen}"},
              "monitor": ["freertos"], "about": "FreeRTOS blinky on STM32F411 (example %s)" % example})
    return s


PICO = {"pico": ("pico", None), "pico2": ("pico2", "rp2350-arm-s"), "zero": ("waveshare_rp2040_zero", None),
        "rp2350-zero": ("waveshare_rp2350_zero", "rp2350-arm-s")}


def spec_pico(name, target, example, d):
    target = target or "pico2"
    example = example or "blink"
    if target not in PICO:
        die("pico target must be one of %s (RISC-V variants: use mcu build)" % ", ".join(PICO))
    board, plat = PICO[target]
    exdir = example_src(MCU_ROOT / "pico/pico-examples" / ("hello_world/usb" if example == "hello_usb" else example))
    if not (exdir / "CMakeLists.txt").exists():
        die("pico example not found: %s" % exdir)
    copytree(exdir, d / "app")
    shutil.copy2(MCU_ROOT / "pico/pico-sdk/external/pico_sdk_import.cmake", d / "pico_sdk_import.cmake")
    tgt = re.search(r"add_executable\(\s*([A-Za-z0-9_]+)", (exdir / "CMakeLists.txt").read_text()).group(1)
    (d / "CMakeLists.txt").write_text("""cmake_minimum_required(VERSION 3.13)
{host}
# Firmware: Pico SDK (PICO_SDK_PATH, PICO_BOARD, PICO_PLATFORM come from the CMake profile).
include(pico_sdk_import.cmake)
project({name} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()
function(example_auto_set_url target)
endfunction()
add_subdirectory(app)   # copy of pico-examples/{ex} (target {tgt})
""".format(name=name, host=HOST_BLOCK.format(name=name), ex=example, tgt=tgt))
    opts = ["-DPICO_BOARD=%s" % board, "-DPICO_SDK_PATH=%s" % (MCU_ROOT / "pico/pico-sdk"),
            "-DPICO_TOOLCHAIN_PATH=%s" % (MCU_ROOT / "tools/toolchains/arm-gnu-toolchain"),
            "-Dpicotool_DIR=%s" % (MCU_ROOT / "pico/picotool")]
    if plat:
        opts.append("-DPICO_PLATFORM=%s" % plat)
    cfg = "target/rp2350.cfg" if plat else "target/rp2040.cfg"
    return {
        "kind": "cmake", "env_sdk": "pico", "options": opts, "cmake_project": name,
        "target": tgt, "elf": "app/%s.elf" % tgt,
        # the plugin runs `find $MCU_OUT/<target> -name <ex>.uf2`, which does not enter a symlinked start
        # dir, so the artifacts are copied into a real staging dir
        "flash": {"how": "mcu", "sdk": "pico", "target": target, "example": tgt, "stage_level": "target",
                  "stage": "{gen}", "stage_copy": ["app/%s.uf2" % tgt, "app/%s.elf" % tgt]},
        "monitor": ["pico"],
        "debug": openocd_debug("OpenOCD + CMSIS-DAP (debugprobe)", "interface/cmsis-dap.cfg", cfg,
                               extra=("-c", "adapter speed 5000")),
        "debug_skip": None, "about": "Pico SDK %s for %s" % (example, board),
    }


def spec_esp(name, target, example, d):
    target = target or "esp32s3"
    example = example or "get-started/blink"
    src = example_src(MCU_ROOT / "esp-idf/esp-idf/examples" / example)
    if not (src / "CMakeLists.txt").exists():
        die("esp-idf example not found: %s" % src)
    copytree(src, d, ignore=("pytest_*", "sdkconfig", "managed_components", "dependencies.lock"))
    if (d / "README.md").exists():
        (d / "README.md").rename(d / "README.example.md")
    proj = cmake_project_name(d / "CMakeLists.txt")
    add_host_block(d / "CMakeLists.txt", name)
    return {
        "kind": "cmake", "env_sdk": "esp-idf",
        "options": ["-DIDF_TARGET=%s" % target, "-DSDKCONFIG={gen}/sdkconfig", "-DCCACHE_ENABLE=0"],
        "cmake_project": proj, "target": "app", "elf": proj + ".elf",
        "flash": {"how": "esptool", "sdk": "esp-idf", "chip": target},
        "monitor": ["esp-idf"],
        # idf.py monitor would re-run cmake on CLion's build dir; call the IDF monitor on the ELF instead
        "monitor_cmd": ["run", "esp-idf", "--", "python", "-m", "esp_idf_monitor", "--target", target,
                        "--toolchain-prefix", "xtensa-%s-elf-" % target if not target.startswith("esp32c")
                        and not target.startswith("esp32p") else "riscv32-esp-elf-", "{port:-p}", "{elf}"],
        "debug": None,
        "debug_skip": "Xtensa core: CLion's bundled GDB has no Xtensa support; use 'idf.py openocd' + "
                      "xtensa-esp32s3-elf-gdb from the IDF tools (mcu shell esp-idf)",
        "about": "ESP-IDF %s for %s" % (example, target),
    }


def spec_k210(name, target, example, d):
    target = target or "cm1"
    example = example or "hello_world"
    sdk = MCU_ROOT / "k210/kendryte-standalone-sdk"
    src = sdk / "src" / example
    if not src.is_dir():
        src = MCU_ROOT / "k210/kendryte-standalone-demo" / example
    src = example_src(src)
    if not src.is_dir():
        die("k210 example not found: %s" % example)
    copytree(src, d / "src")
    (d / "CMakeLists.txt").write_text("""cmake_minimum_required(VERSION 3.5)
{host}
# Firmware: kendryte-standalone-sdk's own CMake (common.cmake, lib/, executable.cmake), used out of tree.
# SDK_ROOT and TOOLCHAIN come from the CMake profile.
set(PROJ {name})
set(BUILDING_SDK "yes" CACHE INTERNAL "")
include(${{SDK_ROOT}}/cmake/common.cmake)
project(${{PROJ}} C CXX ASM)
include(${{SDK_ROOT}}/cmake/macros.internal.cmake)
header_directories(${{SDK_ROOT}}/lib)
header_directories(${{CMAKE_CURRENT_LIST_DIR}}/src)
add_subdirectory(${{SDK_ROOT}}/lib kendryte)
add_source_files(src/*.c src/*.s src/*.S src/*.cpp)
include(${{SDK_ROOT}}/cmake/executable.cmake)
""".format(name=name, host=HOST_BLOCK.format(name=name)))
    return {
        "kind": "cmake", "env_sdk": "k210",
        "options": ["-DSDK_ROOT=%s" % sdk, "-DTOOLCHAIN=%s" % (MCU_ROOT / "k210/toolchain/kendryte-toolchain/bin"),
                    "-DCMAKE_POLICY_VERSION_MINIMUM=3.5"],
        "cmake_project": name, "target": name, "elf": name,
        "flash": {"how": "mcu", "sdk": "k210", "target": target, "example": name, "stage": "{gen}"},
        "monitor": ["k210"], "debug": None,
        "debug_skip": "the workspace has no K210-capable OpenOCD (needs kendryte's fork + a JTAG adapter)",
        "about": "K210 standalone SDK %s" % example,
    }


def _arch_toolchain(d, system, proc, cc, cxx=None, glob=None):
    lines = ["# cross compiler for CLion's code insight (firmware-index); generated by mcu-ide",
             "set(CMAKE_SYSTEM_NAME %s)" % system, "set(CMAKE_SYSTEM_PROCESSOR %s)" % proc,
             'get_filename_component(_MCU_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../tool" ABSOLUTE)']
    if glob:
        lines += ['file(GLOB _bin LIST_DIRECTORIES true "${_MCU_ROOT}/%s")' % glob,
                  "list(SORT _bin)", "list(GET _bin -1 _bin)"]
        cc = "${_bin}/" + cc
        cxx = "${_bin}/" + cxx if cxx else None
    else:
        cc = "${_MCU_ROOT}/" + cc
        cxx = "${_MCU_ROOT}/" + cxx if cxx else None
    lines.append('set(CMAKE_C_COMPILER "%s")' % cc)
    if cxx:
        lines.append('set(CMAKE_CXX_COMPILER "%s")' % cxx)
    lines.append("set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)")
    (d / "cmake").mkdir(exist_ok=True)
    (d / "cmake/toolchain.cmake").write_text("\n".join(lines) + "\n")


def _make_cmakelists(name, langs, body):
    return """cmake_minimum_required(VERSION 3.20)
{host}
# Firmware: built by the SDK's own make (CLion Build = target "firmware").
# Code insight: target "firmware-index" holds the SDK's real compile flags (never built).
project({name} {langs})
if(NOT MCU_ROOT)
  get_filename_component(MCU_ROOT "${{CMAKE_CURRENT_LIST_DIR}}/../../tool" ABSOLUTE)
endif()
include("${{MCU_ROOT}}/clion/cmake/McuProject.cmake")
set(MCU_FW_MAKE_ARGS "" CACHE STRING "extra make arguments (set per CMake profile)")
set(FW "${{CMAKE_BINARY_DIR}}/fw")
{body}""".format(host=HOST_BLOCK.format(name=name), name=name, langs=langs, body=body)


WCH = {"ch32v003": ("CH32V003", "examples"), "ch32v203": ("CH32V203C8T6", "examples_v20x"),
       "ch32v307": ("CH32V307", "examples_v30x")}


def spec_wch(name, target, example, d):
    target = target or "ch32v003"
    example = example or "blink"
    if target not in WCH:
        die("wch target must be one of %s" % ", ".join(WCH))
    mcu, base = WCH[target]
    src = example_src(MCU_ROOT / "wch/ch32fun" / base / example)
    mk = (src / "Makefile").read_text()
    tname = re.search(r"^TARGET\s*:?=\s*(\S+)", mk, re.M).group(1)
    ext = (re.search(r"^TARGET_EXT\s*:?=\s*(\S+)", mk, re.M) or [None, "c"])[1]
    app = d / "app"
    app.mkdir(parents=True)
    for f in src.iterdir():
        if f.suffix in (".c", ".h", ".cpp") and f.is_file():
            shutil.copy2(f, app / f.name)
    shutil.copy2(src / "Makefile", app / "Makefile.example")
    (app / "Makefile").write_text("""# {name}: ch32fun example '{ex}' built with ch32fun's own ch32fun.mk, out of the SDK tree.
# Every output (elf/bin/hex/map/lst, generated linker script) goes to the current directory:
#   make -C <outdir> -f <project>/app/Makefile build        (CLion: <build dir>/fw)
# Makefile.example is the untouched original from {src}.
APP_DIR := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
MCU_ROOT ?= $(abspath $(APP_DIR)/../../../tool)
CH32FUN_REPO ?= $(MCU_ROOT)/wch/ch32fun
TARGET := {t}
TARGET_EXT := {ext}
TARGET_MCU ?= {mcu}
# ch32fun.mk regenerates its linker script on every build; keep it here instead of in the SDK
override GENERATED_LD_FILE := $(CURDIR)/generated_$(TARGET_MCU).ld
vpath %.c $(APP_DIR)
vpath %.cpp $(APP_DIR)
include $(CH32FUN_REPO)/ch32fun/ch32fun.mk
FILES_TO_COMPILE := $(SYSTEM_C) $(APP_DIR)/$(TARGET).$(TARGET_EXT) $(ADDITIONAL_C_FILES)
CFLAGS += -I$(APP_DIR)
clean : cv_clean
""".format(name=name, ex=example, src=src.relative_to(MCU_ROOT), t=tname, ext=ext, mcu=mcu))
    _arch_toolchain(d, "Generic", "riscv", "wch/toolchain/xpack-riscv-none-elf-gcc/bin/riscv-none-elf-gcc")
    make = ('"${{MCU_MAKE}}" -C "${{FW}}" -f "${{CMAKE_CURRENT_SOURCE_DIR}}/app/Makefile" '
            'TARGET_MCU={mcu} TARGET_MCU_PACKAGE={mcu} ${{MCU_FW_MAKE_ARGS}}').format(mcu=mcu)
    body = """mcu_firmware(
  OUTPUT_DIR "${{FW}}"
  BUILD {make} build -j${{MCU_JOBS}}
  ARTIFACTS "${{FW}}/{t}.elf" "${{FW}}/{t}.bin" "${{FW}}/{t}.hex"
  INDEX_DRY_RUN {make} -n -B -w build
  CONFIGURE_DEPENDS "${{CMAKE_CURRENT_SOURCE_DIR}}/app/Makefile")
""".format(make=make, t=tname)
    (d / "CMakeLists.txt").write_text(_make_cmakelists(name, "C", body))
    return {
        "kind": "make", "env_sdk": "wch", "options": [], "toolchain": "cmake/toolchain.cmake",
        "cmake_project": name, "target": "firmware", "elf": "fw/%s.elf" % tname,
        "flash": {"how": "mcu", "sdk": "wch", "target": target, "example": example, "stage": "{gen}/fw"},
        "monitor": ["wch"],
        "debug": {"label": "minichlink GDB server (WCH-LinkE)", "exe": str(MCU_ROOT / "wch/bin/minichlink"),
                  "args": ["-baG"], "connect": "tcp:localhost:3333", "reset": "monitor reset halt"},
        "debug_skip": None, "about": "ch32fun %s for %s (make)" % (example, mcu),
    }


PY32 = {"py32f003x8": "PY32F003x8", "py32f002ax5": "PY32F002Ax5", "py32f030x8": "PY32F030x8"}


def spec_py32(name, target, example, d):
    target = target or "py32f003x8"
    example = example or "led-toggle"
    if target not in PY32:
        die("py32 target must be one of %s" % ", ".join(PY32))
    tpl = MCU_ROOT / "py32/py32f0-template"
    exrel = "PY32F0xx/LL/GPIO/LED_Toggle" if example == "led-toggle" else example
    src = example_src(tpl / "Examples" / exrel)
    if not src.is_dir():
        die("py32 example not found: %s" % src)
    copytree(src, d / "app")
    ll = "n" if "/HAL/" in exrel else "y"
    _arch_toolchain(d, "Generic", "arm", "tools/toolchains/arm-gnu-toolchain/bin/arm-none-eabi-gcc")
    make = ('"${{MCU_MAKE}}" -C "${{PY32_STAGE}}" MCU_TYPE={m} USE_LL_LIB={ll} APP_DIR=app '
            '"BDIR=${{FW}}" "ARM_TOOLCHAIN=${{MCU_ROOT}}/tools/toolchains/arm-gnu-toolchain/bin" IS_GCC_ABOVE_12=1 '
            '${{MCU_FW_MAKE_ARGS}}').format(m=PY32[target], ll=ll)
    body = """# py32f0-template's Makefile uses paths relative to its own directory, so it runs in a stage dir
# inside the build dir: symlinks to every template entry plus app -> this project's app/.
# Objects and app.elf/bin/hex go to BDIR = <build dir>/fw; the template tree is only read.
set(PY32_TEMPLATE "${{MCU_ROOT}}/py32/py32f0-template")
set(PY32_STAGE "${{CMAKE_BINARY_DIR}}/py32-stage")
file(MAKE_DIRECTORY "${{PY32_STAGE}}")
file(GLOB _tpl LIST_DIRECTORIES true RELATIVE "${{PY32_TEMPLATE}}" "${{PY32_TEMPLATE}}/*")
foreach(_e IN LISTS _tpl)
  if(NOT _e MATCHES "^(Build|app|[.].*)$")
    file(CREATE_LINK "${{PY32_TEMPLATE}}/${{_e}}" "${{PY32_STAGE}}/${{_e}}" SYMBOLIC)
  endif()
endforeach()
file(CREATE_LINK "${{CMAKE_CURRENT_SOURCE_DIR}}/app" "${{PY32_STAGE}}/app" SYMBOLIC)
mcu_firmware(
  OUTPUT_DIR "${{FW}}"
  BUILD {make} -j${{MCU_JOBS}}
  ARTIFACTS "${{FW}}/app.elf" "${{FW}}/app.bin" "${{FW}}/app.hex"
  INDEX_DRY_RUN {make} -n -B -w
  INDEX_ARGS --exclude "\\\\.s$")
""".format(make=make)
    (d / "CMakeLists.txt").write_text(_make_cmakelists(name, "C", body))
    pack = tpl / "Misc/Puya.PY32F0xx_DFP.1.1.7.pack"
    return {
        "kind": "make", "env_sdk": "py32", "options": [], "toolchain": "cmake/toolchain.cmake",
        "profile_options": {"Debug": ["-DMCU_FW_MAKE_ARGS=OPT=-Og"], "Release": []},
        "cmake_project": name, "target": "firmware", "elf": "fw/app.elf",
        "flash": {"how": "mcu", "sdk": "py32", "target": target, "example": example, "stage": "{gen}/fw"},
        "monitor": ["py32"],
        "debug": {"label": "pyOCD GDB server", "exe": str(MCU_ROOT / "py32/.venv/bin/pyocd"),
                  "args": ["gdbserver", "--pack", str(pack), "-t", target, "--port", "3333"],
                  "connect": "tcp:localhost:3333", "reset": "monitor reset halt"},
        "debug_skip": None, "about": "py32f0-template %s for %s (make)" % (exrel, PY32[target]),
    }


AVR = {"nano": "arduino:avr:nano:cpu=atmega328", "nano-old": "arduino:avr:nano:cpu=atmega328old",
       "mega": "arduino:avr:mega:cpu=atmega2560", "attiny85": "ATTinyCore:avr:attinyx5:chip=85,clock=8internal"}


def spec_avr(name, target, example, d):
    target = target or "nano"
    example = example or "Blink"
    fqbn = AVR.get(target, target)
    src = example_src(MCU_ROOT / "avr/sketches" / example)
    if not src.is_dir():
        die("avr sketch not found: %s" % src)
    sk = d / example
    sk.mkdir(parents=True)
    for f in src.iterdir():
        if f.is_file() and f.suffix in (".ino", ".h", ".c", ".cpp"):
            shutil.copy2(f, sk / f.name)
    _arch_toolchain(d, "Generic", "avr", "avr-gcc", "avr-g++", glob="avr/data/packages/arduino/tools/avr-gcc/*/bin")
    cli = ('"${{MCU_ROOT}}/avr/bin/arduino-cli" compile -b {fqbn} --build-path "${{CMAKE_BINARY_DIR}}/arduino-build"'
           ' "${{CMAKE_CURRENT_SOURCE_DIR}}/{ex}"').format(fqbn=fqbn, ex=example)
    body = """# arduino-cli compiles the sketch; --only-compilation-database gives the index flags at configure time.
mcu_firmware(
  OUTPUT_DIR "${{FW}}"
  BUILD {cli} -j ${{MCU_JOBS}} --output-dir "${{FW}}"
  ARTIFACTS "${{FW}}/{ex}.ino.hex" "${{FW}}/{ex}.ino.elf"
  INDEX_PREPARE {cli} --only-compilation-database
  INDEX_COMPDB "${{CMAKE_BINARY_DIR}}/arduino-build/compile_commands.json"
  INDEX_ARGS --map "{ex}\\\\.ino\\\\.cpp$=${{CMAKE_CURRENT_SOURCE_DIR}}/{ex}/{ex}.ino"
  CONFIGURE_DEPENDS "${{CMAKE_CURRENT_SOURCE_DIR}}/{ex}/{ex}.ino")
""".format(cli=cli, ex=example)
    (d / "CMakeLists.txt").write_text(_make_cmakelists(name, "C CXX", body))
    return {
        "kind": "make", "env_sdk": "avr", "options": [], "toolchain": "cmake/toolchain.cmake",
        "cmake_project": name, "target": "firmware", "elf": "fw/%s.ino.elf" % example,
        "flash": {"how": "mcu", "sdk": "avr", "target": target, "example": example, "stage": "{gen}/fw"},
        "monitor": ["avr"], "debug": None,
        "debug_skip": "AVR: no debugWIRE/JTAG GDB server in the workspace (Nano has no debug probe)",
        "about": "Arduino sketch %s for %s (arduino-cli)" % (example, fqbn),
    }


BL = {"bl616": ("bl616", "bl616dk", "target/tgt_e907.cfg"), "bl702": ("bl702", "bl702dk", "target/tgt_702.cfg")}


def spec_bouffalo(name, target, example, d):
    target = target or "bl616"
    example = example or "helloworld"
    if target not in BL:
        die("bouffalo target must be one of %s (BL808: use mcu build)" % ", ".join(BL))
    chip, board, ocd = BL[target]
    sdk = MCU_ROOT / "bouffalo/bouffalo_sdk"
    copytree(example_src(sdk / "examples" / example), d / "app")
    _arch_toolchain(d, "Generic", "riscv",
                    "bouffalo/toolchain/xuantie-gcc-riscv-v2.10.2-linux-amd64/bin/riscv64-unknown-elf-gcc")
    make = ('"${{CMAKE_COMMAND}}" -E env CMAKE_EXPORT_COMPILE_COMMANDS=ON '
            '"${{MCU_MAKE}}" -C "${{CMAKE_CURRENT_SOURCE_DIR}}/app" CHIP={c} BOARD={b} "BUILD_DIR=${{FW_REL}}" '
            '${{MCU_FW_MAKE_ARGS}}').format(c=chip, b=board)
    body = """# bouffalo_sdk's project.build (Kconfig seed + inner CMake) runs in app/ with BUILD_DIR = <build dir>/fw.
# Its inner CMake writes compile_commands.json, which feeds the index. The SDK joins SDK_DEMO_PATH and
# BUILD_DIR, so BUILD_DIR is passed relative to app/.
file(RELATIVE_PATH FW_REL "${{CMAKE_CURRENT_SOURCE_DIR}}/app" "${{FW}}")
mcu_firmware(
  OUTPUT_DIR "${{FW}}"
  BUILD {make}
  ARTIFACTS "${{FW}}/build_out/{ex}_{c}.bin" "${{FW}}/build_out/{ex}_{c}.elf"
  INDEX_PREPARE {make} cmake_cache
  INDEX_COMPDB "${{FW}}/compile_commands.json"
  CONFIGURE_DEPENDS "${{CMAKE_CURRENT_SOURCE_DIR}}/app/defconfig" "${{CMAKE_CURRENT_SOURCE_DIR}}/app/CMakeLists.txt")
""".format(make=make, ex=example, c=chip)
    (d / "CMakeLists.txt").write_text(_make_cmakelists(name, "C", body))
    ocd_dir = sdk / "tools/openocd"
    return {
        "kind": "make", "env_sdk": "bouffalo", "options": [], "toolchain": "cmake/toolchain.cmake",
        "cmake_project": name, "target": "firmware", "elf": "fw/build_out/%s_%s.elf" % (example, chip),
        "flash": {"how": "bouffalo", "sdk": "bouffalo", "target": target, "chip": chip, "board": board,
                  "example": example},
        "monitor": ["bouffalo"],
        "debug": {"label": "OpenOCD (bouffalo_sdk) + RV-Debugger Plus / FT2232",
                  "exe": str(ocd_dir / "bin/openocd"),
                  "args": ["-s", str(ocd_dir), "-f", "interface/ft2232d.cfg", "-f", ocd],
                  "connect": "tcp:localhost:3333", "reset": "monitor reset halt", "check": ["-c", "shutdown"]},
        "debug_skip": None, "about": "bouffalo_sdk %s for %s/%s (make)" % (example, chip, board),
    }


def spec_k230(name, target, example, d):
    target = target or "k230d_rtos_bpi_zero"
    example = example or "hello"
    sdk = MCU_ROOT / "k230/sdk"
    src = example_src(sdk / "src/applications/helloworld")
    app = d / "app"
    app.mkdir(parents=True)
    for f in src.iterdir():
        if f.suffix in (".c", ".cc", ".cpp", ".h") and f.is_file():
            shutil.copy2(f, app / f.name)
    orig = (src / "Makefile").read_text()
    shutil.copy2(src / "Makefile", app / "Makefile.example")
    # same rules as the SDK's helloworld Makefile; only the SDK location and the output dirs differ
    mk = orig.replace("include ../mkenv.mk\n", """MKENV_DIR := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
MCU_ROOT ?= $(abspath $(MKENV_DIR)/../../../tool)
export SDK_SRC_ROOT_DIR := $(realpath $(MCU_ROOT)/k230/sdk)
include $(SDK_SRC_ROOT_DIR)/tools/mkenv.mk
include $(SDK_SRC_ROOT_DIR)/.config
""")
    mk = mk.replace("DIR \t:= $(shell basename `pwd`)", "DIR \t:= {name}".format(name=name))
    mk = mk.replace("BUILD\t:= $(SDK_APPS_BUILD_DIR)/$(DIR)", "OUT\t?= $(CURDIR)/out\nBUILD\t:= $(OUT)/obj")
    mk = mk.replace("BIN \t:= $(SDK_APPS_IMAGE_DIR)/$(DIR)", "BIN \t:= $(OUT)/$(DIR)")
    mk = mk.replace("SRC_DIRS  := .", "SRC_DIRS  := $(MKENV_DIR)")
    mk = mk.replace("-I. $(LIB_CFLAGS)", "-I$(MKENV_DIR) $(LIB_CFLAGS)")
    for must in ("SRC_SRC_ROOT", "$(OUT)/obj", "$(OUT)/$(DIR)", "$(MKENV_DIR)"):
        if must == "SRC_SRC_ROOT":
            continue
        if must not in mk:
            die("k230: the SDK's helloworld Makefile changed; adapt spec_k230 (missing %s)" % must)
    (app / "Makefile").write_text(
        "# %s: copy of the SDK's src/applications/helloworld/Makefile (original: Makefile.example).\n"
        "# Same rules and SDK .mk includes; sources from this dir, outputs in OUT (CLion: <build dir>/fw).\n"
        "#   make -C <project>/app OUT=<dir>\n" % name + mk)
    _arch_toolchain(d, "Generic", "riscv64",
                    "k230/toolchain/riscv64-linux-musleabi_for_x86_64-pc-linux-gnu/bin/riscv64-unknown-linux-musl-gcc")
    make = ('"${MCU_MAKE}" -C "${CMAKE_CURRENT_SOURCE_DIR}/app" "OUT=${FW}" ${MCU_FW_MAKE_ARGS}')
    body = """# RT-Smart user app, built with the K230 SDK's own .mk files (tools/mkenv.mk, .config, libs/mk/*.mk).
# The SDK tree is only read; build products go to <build dir>/fw.
mcu_firmware(
  OUTPUT_DIR "${{FW}}"
  BUILD {make} all
  ARTIFACTS "${{FW}}/{name}"
  INDEX_DRY_RUN {make} -n -B -w all
  CONFIGURE_DEPENDS "${{CMAKE_CURRENT_SOURCE_DIR}}/app/Makefile")
""".format(make=make, name=name)
    (d / "CMakeLists.txt").write_text(_make_cmakelists(name, "C CXX", body))
    return {
        "kind": "make", "env_sdk": "k230", "options": [], "toolchain": "cmake/toolchain.cmake",
        "cmake_project": name, "target": "firmware", "elf": "fw/" + name,
        "flash": {"how": "k230-deploy", "sdk": "k230", "target": target},
        "monitor": ["k230"], "debug": None,
        "debug_skip": "RT-Smart user process on a C908 running an OS: needs an on-target gdbserver that the "
                      "SDK image does not ship; JTAG debug of the kernel is out of scope",
        "about": "K230 RT-Smart user app (SDK helloworld, musl) for %s" % target,
    }


SPECS = {"zephyr": spec_zephyr, "stm32": spec_stm32, "freertos": spec_freertos, "pico": spec_pico,
         "esp-idf": spec_esp, "k210": spec_k210, "wch": spec_wch, "py32": spec_py32, "avr": spec_avr,
         "bouffalo": spec_bouffalo, "k230": spec_k230}


# ----------------------------------------------------------------------------------- XML writers
def write_xml(path, root):
    ET.indent(root, space="  ")
    path.parent.mkdir(parents=True, exist_ok=True)
    data = ET.tostring(root, encoding="unicode")
    path.write_text('<?xml version="1.0" encoding="UTF-8"?>\n' + data + "\n" if root.tag == "project"
                    else data + "\n")


def gen_dir(name, profile):
    return BUILD_ROOT / name / ("cmake-build-" + profile.lower())


def cmake_xml(proj_dir, name, spec):
    fw_env = sdk_env(spec["env_sdk"])
    host_env = sdk_env("host")
    root = ET.Element("project", version="4")
    comp = ET.SubElement(root, "component", name="CMakeSharedSettings")
    confs = ET.SubElement(comp, "configurations")
    base = ["-G", "Ninja", "-DCMAKE_MAKE_PROGRAM=%s" % HOST_NINJA]
    for prof in PROFILES:
        gd = gen_dir(name, prof)
        if prof == "Host-Tests":
            opts = base + ["-DMCU_HOST_TESTS=ON", "-DCMAKE_C_COMPILER=%s" % (HOST_BT / "gcc"),
                           "-DCMAKE_CXX_COMPILER=%s" % (HOST_BT / "g++")]
            env, cfg = host_env, "Debug"
        else:
            opts = list(base)
            if spec.get("toolchain"):
                opts.append("-DCMAKE_TOOLCHAIN_FILE=%s" % (proj_dir / spec["toolchain"]))
            opts += [o.replace("{gen}", str(gd)) for o in spec.get("options", [])]
            opts += spec.get("profile_options", {}).get(prof, [])
            env, cfg = fw_env, prof
        c = ET.SubElement(confs, "configuration", PROFILE_NAME=prof, ENABLED="true", GENERATION_DIR=str(gd),
                          CONFIG_NAME=cfg, TOOLCHAIN_NAME="Default",
                          GENERATION_OPTIONS=" ".join(shlex.quote(o) for o in opts),
                          BUILD_OPTIONS="-j %d" % JOBS)
        envs = ET.SubElement(ET.SubElement(c, "ADDITIONAL_GENERATION_ENVIRONMENT"), "envs")
        for k, v in env.items():
            ET.SubElement(envs, "env", name=k, value=v)
    write_xml(proj_dir / ".idea/cmake.xml", root)
    mroot = ET.Element("project", version="4")
    ET.SubElement(mroot, "component", name="CMakeWorkspace", PROJECT_DIR="$PROJECT_DIR$")
    write_xml(proj_dir / ".idea/misc.xml", mroot)


def run_config(name, ctype, folder, **attrs):
    root = ET.Element("component", name="ProjectRunConfigurationManager")
    a = {"default": "false", "name": name, "type": ctype}
    if folder:
        a["folderName"] = folder
    a.update(attrs)
    return root, ET.SubElement(root, "configuration", a)


def sh_config(path, name, folder, script, terminal=False):
    """Shell Script run configuration (type ShConfigurationType), script text run by /bin/bash in $PROJECT_DIR$."""
    root, c = run_config(name, "ShConfigurationType", folder)
    for k, v in (("SCRIPT_TEXT", script), ("INDEPENDENT_SCRIPT_PATH", "true"), ("SCRIPT_PATH", ""),
                 ("SCRIPT_OPTIONS", ""), ("INDEPENDENT_SCRIPT_WORKING_DIRECTORY", "true"),
                 ("SCRIPT_WORKING_DIRECTORY", "$PROJECT_DIR$"), ("INDEPENDENT_INTERPRETER_PATH", "true"),
                 ("INTERPRETER_PATH", "/bin/bash"), ("INTERPRETER_OPTIONS", ""),
                 ("EXECUTE_IN_TERMINAL", "true" if terminal else "false"), ("EXECUTE_SCRIPT_FILE", "false")):
        ET.SubElement(c, "option", name=k, value=v)
    ET.SubElement(c, "envs")
    ET.SubElement(c, "method", v="2")
    write_xml(path, root)


def gdb_config(path, name, spec, proj_dir):
    """Embedded GDB Server run configuration with CLion's bundled (multiarch) GDB."""
    dbg = spec["debug"]
    attrs = {"factoryName": "com.jetbrains.cidr.embedded.customgdbserver.factory",
             # CLion splits this with ParametersListUtil: double quotes only
             "PROGRAM_PARAMS": " ".join('"%s"' % x if re.search(r"\s", x) else x for x in dbg["args"]),
             "REDIRECT_INPUT": "false", "ELEVATE": "false", "USE_EXTERNAL_CONSOLE": "false",
             "EMULATE_TERMINAL": "false", "WORKING_DIR": "file://$PROJECT_DIR$", "PASS_PARENT_ENVS_2": "true",
             "PROJECT_NAME": spec["cmake_project"], "TARGET_NAME": spec["target"], "CONFIG_NAME": "Debug",
             "version": "1"}
    if spec["kind"] == "make":
        attrs["RUN_PATH"] = str(gen_dir(proj_dir.name, "Debug") / spec["elf"])
    else:
        attrs["RUN_TARGET_PROJECT_NAME"] = spec["cmake_project"]
        attrs["RUN_TARGET_NAME"] = spec["target"]
    root, c = run_config(name, "com.jetbrains.cidr.embedded.customgdbserver.type", "Firmware", **attrs)
    s = ET.SubElement(c, "custom-gdb-server", {"version": "1", "gdb-connect": dbg["connect"],
                                               "executable": dbg["exe"], "warmup-ms": "500",
                                               "download-type": "ALWAYS", "reset-cmd": dbg["reset"],
                                               "reset-type": "AFTER_DOWNLOAD"})
    ET.SubElement(s, "debugger", kind="GDB", isBundled="true")
    m = ET.SubElement(c, "method", v="2")
    ET.SubElement(m, "option", name="CLION.COMPOUND.BUILD", enabled="true")
    write_xml(path, root)


def ctest_config(path, name, spec):
    attrs = {"factoryName": "CTestRun", "PROGRAM_PARAMS": "--output-on-failure", "REDIRECT_INPUT": "false",
             "ELEVATE": "false", "USE_EXTERNAL_CONSOLE": "false", "EMULATE_TERMINAL": "false",
             "PASS_PARENT_ENVS_2": "true", "PROJECT_NAME": spec["name"] + "_host_tests",
             "TARGET_NAME": "All CTest", "CONFIG_NAME": "Host-Tests", "TEST_MODE": "SUITE_TEST"}
    root, c = run_config(name, "CTestRunConfiguration", "Tests", **attrs)
    m = ET.SubElement(c, "method", v="2")
    ET.SubElement(m, "option", name="CLION.COMPOUND.BUILD", enabled="true")
    write_xml(path, root)


# ------------------------------------------------------------------------------------- generator
README = """# {name}

{about}. Generated by `mcu/clion/mcu-ide new {sdk} {name} {target}{ex}` (`mcu/clion/new-project`); the SDK
example was **copied** here, the SDK tree under `mcu/{sdk}` is never modified.

## Open in CLion
File > Open > `mcu/projects/{name}` (this folder). CLion reads `.idea/cmake.xml` and loads three CMake
profiles; pick one in the toolbar:

| Profile | What it builds | Build dir |
|---|---|---|
| Debug | firmware{dbgnote} | `{gd}` |
| Release | firmware | `{gr}` |
| Host-Tests | `tests/` with the buildtools gcc + GoogleTest (CTest) | `{gh}` |

Each profile's environment (PATH = the SDK's `env.sh` tools + `mcu/host/sysbin`, plus the SDK variables)
and CMake options live in `.idea/cmake.xml`; nothing is taken from the IDE's global toolchain except
CLion's bundled CMake. The Build button (Ctrl+F9) builds {build_what}.

## Run configurations (`.run/`, folder in the run widget)
| Name | Type | Does |
|---|---|---|
| Build (mcu-ide) | Shell Script | `../../ide/clion/mcu-ide build . Debug`: the same configure + build from a shell |
| Flash | Shell Script | builds, then `{flash_cmd}` on this project's output |
| Monitor | Shell Script (terminal) | `mcu monitor {sdk}` (serial console; edit the port in the script text) |
{debug_row}| Host tests | CTest | runs `tests/` (select the Host-Tests profile first) |

## Files
- `CMakeLists.txt`: firmware build{cm_note}; the `MCU_HOST_TESTS` block at the top switches to the host tests.
- `lib/ringbuf/`: example of target-independent code; `tests/test_ringbuf.cpp` tests it on the host.
- `mcu-project.json`: what `mcu-ide` needs (SDK, target, flash and debug settings). Edit it, then run
  `../../ide/clion/mcu-ide new ... --force` only if you want the run configs regenerated.
- Command line equivalents: `../../ide/clion/mcu-ide build|test|flash|monitor|gdbserver {name}`.
"""


def write_project(sdk, name, target, example, force=False):
    if sdk not in SPECS:
        die("unsupported sdk '%s' (have: %s)" % (sdk, " ".join(SPECS)))
    if not re.match(r"^[A-Za-z0-9][A-Za-z0-9_.-]*$", name):
        die("project name must be [A-Za-z0-9_.-]")
    d = PROJECTS / name
    if d.exists():
        if not force:
            die("%s exists (use --force to regenerate its CLion files; sources are kept)" % d)
        trash = Path.home() / ".local/share/Trash/files"
        trash.mkdir(parents=True, exist_ok=True)
        dst = trash / ("mcu-project-%s-%d" % (name, int(__import__("time").time())))
        shutil.move(str(d), str(dst))
        say("moved old %s to %s" % (d, dst))
    d.mkdir(parents=True)
    try:
        spec = SPECS[sdk](name, target, example, d)
    except BaseException:
        shutil.move(str(d), str(Path.home() / ".local/share/Trash/files" / ("mcu-project-%s-failed" % name)))
        raise
    spec.update({"sdk": sdk, "name": name, "sdk_target": target or "", "example": example or ""})
    tpl = CLION_DIR / "templates/common"
    copytree(tpl / "lib", d / "lib")
    copytree(tpl / "tests", d / "tests")
    cmake_xml(d, name, spec)
    run = d / ".run"
    sh_config(run / "Build.run.xml", "Build (mcu-ide, Debug)", "Firmware", "../../ide/clion/mcu-ide build . Debug")
    sh_config(run / "Flash.run.xml", "Flash", "Firmware", "../../ide/clion/mcu-ide flash .")
    sh_config(run / "Monitor.run.xml", "Monitor", "Firmware", "../../ide/clion/mcu-ide monitor .", terminal=True)
    if spec.get("debug"):
        gdb_config(run / "Debug.run.xml", "Debug (%s)" % spec["debug"]["label"], spec, d)
    ctest_config(run / "Host_tests.run.xml", "Host tests", spec)
    (d / ".gitignore").write_text("cmake-build-*/\n.idea/workspace.xml\n.idea/shelf/\nmanaged_components/\n"
                                  "dependencies.lock\n")
    fl = spec["flash"]
    flash_cmd = {"mcu": "mcu flash %s %s %s" % (fl.get("sdk"), fl.get("target"), fl.get("example")),
                 "esptool": "mcu run esp-idf -- python -m esptool ... write-flash @flash_args",
                 "bouffalo": "mcu run bouffalo -- make flash (BLFlashCommand)",
                 "k230-deploy": "print how to copy the ELF to the SD card (no device writes)"}[fl["how"]]
    if spec.get("debug"):
        dbg_row = "| %s | Embedded GDB Server | starts `%s`, loads the ELF with CLion's bundled GDB |\n" % (
            "Debug (%s)" % spec["debug"]["label"], Path(spec["debug"]["exe"]).name)
        dbgnote = " (Debug run config uses this profile)"
    else:
        dbg_row = "| (no Debug config) | | %s |\n" % spec["debug_skip"]
        dbgnote = ""
    (d / "README.md").write_text(README.format(
        name=name, about=spec["about"], sdk=sdk, target=target or "", ex=(" " + example) if example else "",
        gd=gen_dir(name, "Debug"), gr=gen_dir(name, "Release"), gh=gen_dir(name, "Host-Tests"),
        dbgnote=dbgnote, flash_cmd=flash_cmd, debug_row=dbg_row,
        build_what="target `firmware` (it runs the SDK's make; target `firmware-index` only feeds code insight)"
        if spec["kind"] == "make" else "all targets of the active profile",
        cm_note=" (thin CMake around the SDK's make)" if spec["kind"] == "make" else ""))
    keep = {k: v for k, v in spec.items() if k not in ("options", "profile_options")}
    (d / "mcu-project.json").write_text(json.dumps(keep, indent=2) + "\n")
    say("created %s (%s)" % (d, spec["about"]))
    return d


# ----------------------------------------------------------------------------------------- runner
def find_project(p):
    d = Path(p)
    if not (d / "mcu-project.json").exists():
        d = PROJECTS / p
    if not (d / "mcu-project.json").exists():
        die("not an mcu-ide project: %s" % p)
    return d.resolve()


def load(d):
    spec = json.loads((d / "mcu-project.json").read_text())
    return spec


def read_profiles(d):
    root = ET.parse(d / ".idea/cmake.xml").getroot()
    out = {}
    for c in root.iter("configuration"):
        env = {e.get("name"): e.get("value") for e in c.iter("env")}
        out[c.get("PROFILE_NAME")] = {
            "dir": c.get("GENERATION_DIR"), "config": c.get("CONFIG_NAME"),
            "options": shlex.split(c.get("GENERATION_OPTIONS", "")),
            "build": shlex.split(c.get("BUILD_OPTIONS", "")), "env": env,
            "pass_system": c.get("GENERATION_PASS_SYSTEM_ENVIRONMENT", "true") != "false"}
    return out


def profile_env(p):
    env = dict(os.environ) if p["pass_system"] else {}
    env.update(p["env"])
    return env


def pick_cmake(arg):
    if arg:
        return arg
    return str(CLION_CMAKE) if CLION_CMAKE.exists() else str(MCU_ROOT / "host/cmake/bin/cmake")


def cmd_build(d, profile, cmake=None, configure_only=False):
    p = read_profiles(d).get(profile) or die("no profile %s in .idea/cmake.xml" % profile)
    cm = pick_cmake(cmake)
    env = profile_env(p)
    # the command line CLion runs: build type, its bundled ninja + generator, then the profile options
    conf = [cm, "-DCMAKE_BUILD_TYPE=%s" % p["config"]]
    if Path(cm) == CLION_CMAKE:
        conf += ["-DCMAKE_MAKE_PROGRAM=%s" % CLION_NINJA, "-G", "Ninja"]
    conf += p["options"] + ["-S", str(d), "-B", p["dir"]]
    say("+ " + " ".join(shlex.quote(x) for x in conf))
    r = subprocess.run(conf, env=env)
    if r.returncode or configure_only:
        return r.returncode
    b = [cm, "--build", p["dir"]] + p["build"]
    say("+ " + " ".join(shlex.quote(x) for x in b))
    return subprocess.run(b, env=env).returncode


def cmd_test(d, cmake=None):
    rc = cmd_build(d, "Host-Tests", cmake)
    if rc:
        return rc
    p = read_profiles(d)["Host-Tests"]
    ctest = str(Path(pick_cmake(cmake)).with_name("ctest"))
    c = [ctest, "--test-dir", p["dir"], "--output-on-failure", "-j", str(JOBS)]
    say("+ " + " ".join(c))
    return subprocess.run(c, env=profile_env(p)).returncode


def stage(spec, d):
    """Make the dispatcher see this project's build output: MCU_OUT_BASE=<gen>/.mcu-out with
    <sdk>/<target>/<example> -> the profile's artifact dir (the plugin's own layout)."""
    gd = gen_dir(d.name, "Debug")
    fl = spec["flash"]
    src = Path(fl["stage"].replace("{gen}", str(gd)))
    base = gd / ".mcu-out"
    link = base / fl["sdk"] / fl["target"]
    if fl.get("stage_level") != "target":
        link = link / fl["example"]
    link.parent.mkdir(parents=True, exist_ok=True)
    if link.is_symlink() or link.is_file():
        link.unlink()
    if fl.get("stage_copy"):
        link.mkdir(parents=True, exist_ok=True)
        for rel in fl["stage_copy"]:
            shutil.copy2(src / rel, link / Path(rel).name)
    else:
        if link.is_dir():
            shutil.rmtree(link)  # a staging dir of copies inside our own build dir
        link.symlink_to(src)
    return base


def cmd_flash(d, extra, no_build=False):
    spec = load(d)
    fl = spec["flash"]
    if not no_build and fl["how"] != "k230-deploy":
        if cmd_build(d, "Debug"):
            die("build failed")
    gd = gen_dir(d.name, "Debug")
    if fl["how"] == "mcu":
        base = stage(spec, d)
        c = [str(MCU_ROOT / "mcu"), "flash", fl["sdk"], fl["target"], fl["example"]] + extra
        env = dict(os.environ, MCU_OUT_BASE=str(base))
        say("+ MCU_OUT_BASE=%s %s" % (base, " ".join(shlex.quote(x) for x in c)))
        return subprocess.run(c, env=env).returncode
    if fl["how"] == "esptool":
        port = ["-p", extra[0]] if extra else []
        c = [str(MCU_ROOT / "mcu"), "run", "esp-idf", "--", "python", "-m", "esptool", "--chip", fl["chip"]] + \
            port + ["-b", "460800", "--before", "default-reset", "--after", "hard-reset", "write-flash",
                    "@flash_args"]
        say("+ (cd %s) %s" % (gd, " ".join(c)))
        return subprocess.run(c, cwd=gd).returncode
    if fl["how"] == "bouffalo":
        port = extra[0] if extra else "/dev/ttyACM0"
        say("Put the board in boot mode: hold BOOT, tap RESET, then flash over %s" % port)
        c = [str(MCU_ROOT / "mcu"), "run", "bouffalo", "--", "make", "-C", str(d / "app"), "flash",
             "CHIP=" + fl["chip"], "BOARD=" + fl["board"], "BUILD_DIR=" + os.path.relpath(gd / "fw", d / "app"), "COMX=" + port]
        say("+ " + " ".join(shlex.quote(x) for x in c))
        return subprocess.run(c).returncode
    if fl["how"] == "k230-deploy":
        elf = gd / spec["elf"]
        say("K230 RT-Smart apps are not flashed: copy the ELF to the SD card's app dir and run it from the shell.")
        say("  ELF: %s %s" % (elf, "(built)" if elf.exists() else "(not built yet: Build first)"))
        say("  cp %s /run/media/$USER/<SDCARD>/app/ && sync   # then on the board: /sdcard/app/%s" % (elf, d.name))
        say("Full SD image (kernel + this app needs an SDK image build): mcu build k230 %s hello; "
            "mcu flash k230 %s hello /dev/sdX" % (fl["target"], fl["target"]))
        return 0 if elf.exists() else 1
    die("unknown flash method")


def cmd_monitor(d, extra):
    spec = load(d)
    if spec.get("monitor_cmd"):
        elf = str(gen_dir(d.name, "Debug") / spec["elf"])
        c = [str(MCU_ROOT / "mcu")]
        for x in spec["monitor_cmd"]:
            if x == "{port:-p}":
                c += ["-p", extra[0]] if extra else []
            else:
                c.append(x.replace("{elf}", elf))
    else:
        c = [str(MCU_ROOT / "mcu"), "monitor"] + spec["monitor"] + extra
    say("+ " + " ".join(c))
    return subprocess.run(c).returncode


def cmd_gdbserver(d, check=False):
    spec = load(d)
    dbg = spec.get("debug") or die("no debug server for this project: %s" % spec.get("debug_skip"))
    c = [dbg["exe"]] + dbg["args"] + (dbg.get("check", []) if check else [])
    say("+ " + " ".join(shlex.quote(x) for x in c))
    return subprocess.run(c, cwd=d).returncode


def cmd_list():
    for d in sorted(PROJECTS.glob("*/mcu-project.json")):
        s = json.loads(d.read_text())
        print("%-26s %-9s %s" % (d.parent.name, s["sdk"], s["about"]))


# --------------------------------------------------------------------------------------- workspace
def cmd_workspace():
    w = CLION_DIR / "workspace"
    run = w / ".run"
    if run.exists():
        trash = Path.home() / ".local/share/Trash/files"
        trash.mkdir(parents=True, exist_ok=True)
        shutil.move(str(run), str(trash / ("mcu-workspace-run-%d" % int(__import__("time").time()))))
    run.mkdir(parents=True)
    mcu = "../../mcu"
    for fn, nm, sc in (("doctor_all", "doctor all", "doctor all"), ("selfcheck_all", "selfcheck all", "selfcheck all"),
                       ("libcheck_all", "libcheck all", "libcheck all"), ("list", "list SDKs", "list"),
                       ("update_all_dry", "update all (dry run)", "update all")):
        pre = "MCU_YES=0 " if sc.startswith("update") else ""
        sh_config(run / ("ws_%s.run.xml" % fn), nm, "Workspace", "%s%s %s" % (pre, mcu, sc))
    sh_config(run / "ws_projects.run.xml", "list CLion projects", "Workspace", "../mcu-ide list")
    for sdk in sorted(p.parent.name for p in MCU_ROOT.glob("*/sdk.sh")):
        sc = sdk_selfcheck(sdk)
        if not sc:
            sh_config(run / ("%s_doctor.run.xml" % sdk), "%s: doctor" % sdk, sdk, "%s doctor %s" % (mcu, sdk))
            continue
        tgt = " ".join(sc)
        sh_config(run / ("%s_build.run.xml" % sdk), "%s: build %s" % (sdk, tgt), sdk, "%s build %s %s" % (mcu, sdk, tgt))
        sh_config(run / ("%s_flash.run.xml" % sdk), "%s: flash %s" % (sdk, tgt), sdk, "%s flash %s %s" % (mcu, sdk, tgt))
        sh_config(run / ("%s_monitor.run.xml" % sdk), "%s: monitor" % sdk, sdk, "%s monitor %s" % (mcu, sdk),
                  terminal=True)
    root = ET.Element("project", version="4")
    ET.SubElement(root, "component", name="ProjectModuleManager").append(
        ET.Element("modules"))
    mods = root.find("component/modules")
    ET.SubElement(mods, "module", fileurl="file://$PROJECT_DIR$/.idea/mcu-workspace.iml",
                  filepath="$PROJECT_DIR$/.idea/mcu-workspace.iml")
    write_xml(w / ".idea/modules.xml", root)
    iml = ET.Element("module", type="CPP_MODULE", version="4")
    comp = ET.SubElement(iml, "component", name="NewModuleRootManager")
    ET.SubElement(comp, "content", url="file://$MODULE_DIR$/..")
    ET.SubElement(comp, "orderEntry", type="sourceFolder", forTests="false")
    write_xml(w / ".idea/mcu-workspace.iml", iml)
    say("wrote %d run configurations in %s" % (len(list(run.glob("*.xml"))), run))


# ------------------------------------------------------------------------------------------ verify
def verify(d, log):
    """Headless checks for one project; returns dict of results. Never writes to a device."""
    spec = load(d)
    res = {}

    def run(label, cmd, cwd=None, env=None, timeout=1800, ok_codes=(0,)):
        log.write("\n===== %s: %s\n" % (label, cmd if isinstance(cmd, str) else " ".join(map(str, cmd))))
        log.flush()
        try:
            r = subprocess.run(cmd, cwd=cwd, env=env, stdout=log, stderr=subprocess.STDOUT, timeout=timeout,
                               shell=isinstance(cmd, str), executable="/bin/bash" if isinstance(cmd, str) else None)
            return r.returncode
        except subprocess.TimeoutExpired:
            log.write("TIMEOUT\n")
            return 124

    me = [sys.executable, "-I", __file__]
    res["build"] = run("build Debug (CLion cmake)", me + ["build", str(d), "Debug"])
    elf = gen_dir(d.name, "Debug") / spec["elf"]
    res["elf"] = elf.exists()
    res["test"] = run("Host-Tests + ctest", me + ["test", str(d)])
    bad = []
    for x in list(d.glob(".idea/*.xml")) + list(d.glob(".run/*.xml")):
        try:
            ET.parse(x)
        except ET.ParseError as e:
            bad.append("%s: %s" % (x.name, e))
    res["xml"] = "ok" if not bad else bad
    # shell run configs, exactly as CLion runs them (script text, /bin/bash, cwd = project dir);
    # no probe or serial port is attached, so flash/monitor stop when they try to open the device
    sh = {}
    for x in sorted(d.glob(".run/*.xml")):
        c = ET.parse(x).getroot().find("configuration")
        if c.get("type") != "ShConfigurationType":
            continue
        opt = {o.get("name"): o.get("value") for o in c.iter("option")}
        cwd = opt["SCRIPT_WORKING_DIRECTORY"].replace("$PROJECT_DIR$", str(d))
        script = opt["SCRIPT_TEXT"]
        if "mcu-ide build" in script:
            continue  # same as res["build"]
        if "mcu-ide monitor" in script and spec["monitor"] != ["wch"]:
            script += " /dev/ttyMCUIDE-NONE"
        sh[c.get("name")] = run("run config %s" % c.get("name"), "timeout 60 " + script, cwd=cwd, timeout=90)
    res["sh"] = sh
    if spec.get("debug"):
        res["gdbserver"] = run("gdb server check", me + ["gdbserver", str(d), "--check"], timeout=60)
    return res


def cmd_verify(names):
    ds = [find_project(n) for n in names] if names and names != ["all"] else \
        sorted(p.parent for p in PROJECTS.glob("*/mcu-project.json"))
    logdir = BUILD_ROOT / "_verify"
    logdir.mkdir(parents=True, exist_ok=True)
    summary = {}
    for d in ds:
        with open(logdir / (d.name + ".log"), "w") as log:
            say("verify %s (log %s)" % (d.name, log.name))
            summary[d.name] = verify(d, log)
            say("  " + json.dumps(summary[d.name]))
    (logdir / "summary.json").write_text(json.dumps(summary, indent=2))
    return 0


# -------------------------------------------------------------------------------------------- main
def main():
    ap = argparse.ArgumentParser(prog="mcu-ide", description="CLion integration for the mcu workspace",
                                 formatter_class=argparse.RawDescriptionHelpFormatter, epilog=__doc__)
    sub = ap.add_subparsers(dest="cmd", required=True)
    p = sub.add_parser("new")
    p.add_argument("sdk"), p.add_argument("name"), p.add_argument("target", nargs="?"), p.add_argument("example", nargs="?")
    p.add_argument("--force", action="store_true")
    p.add_argument("--source", help="copy the code from DIR instead of the SDK example (template/example hook)")
    sub.add_parser("workspace")
    sub.add_parser("list")
    p = sub.add_parser("build")
    p.add_argument("project"), p.add_argument("profile", nargs="?", default="Debug")
    p.add_argument("--cmake"), p.add_argument("--configure-only", action="store_true")
    p = sub.add_parser("test")
    p.add_argument("project"), p.add_argument("--cmake")
    p = sub.add_parser("flash")
    p.add_argument("project"), p.add_argument("extra", nargs="*"), p.add_argument("--no-build", action="store_true")
    p = sub.add_parser("monitor")
    p.add_argument("project"), p.add_argument("extra", nargs="*")
    p = sub.add_parser("gdbserver")
    p.add_argument("project"), p.add_argument("--check", action="store_true", help="parse the config and exit")
    p = sub.add_parser("verify")
    p.add_argument("projects", nargs="*")
    a = ap.parse_args()
    if a.cmd == "new":
        global SOURCE_OVERRIDE
        SOURCE_OVERRIDE = a.source
        write_project(a.sdk, a.name, a.target, a.example, a.force)
        return 0
    if a.cmd == "workspace":
        return cmd_workspace()
    if a.cmd == "list":
        return cmd_list()
    if a.cmd == "verify":
        return cmd_verify(a.projects)
    d = find_project(a.project)
    if a.cmd in ("flash", "monitor", "gdbserver"):
        # west/esptool/BLFlashCommand are python: don't drop __pycache__ into the SDK trees
        os.environ["PYTHONDONTWRITEBYTECODE"] = "1"
    if a.cmd == "build":
        return cmd_build(d, a.profile, a.cmake, a.configure_only)
    if a.cmd == "test":
        return cmd_test(d, a.cmake)
    if a.cmd == "flash":
        return cmd_flash(d, a.extra, a.no_build)
    if a.cmd == "monitor":
        return cmd_monitor(d, a.extra)
    if a.cmd == "gdbserver":
        return cmd_gdbserver(d, a.check)


if __name__ == "__main__":
    sys.exit(main() or 0)
