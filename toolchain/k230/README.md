# K230 / K230D — RT-Smart + CanMV (non-Linux) SDK

Official **K230 RTOS-only SDK** (`kendryte/k230_rtos_sdk`), assembled with `repo` from
`canmv-k230/manifest`. One tree builds three firmware flavours per board:
`*_rtos_*` (plain RT-Smart + MPP + your apps), `*_canmv_*` (RT-Smart + CanMV MicroPython,
the firmware CanMV IDE talks to) and `*_arduino_*`. It is the newest maintained Canaan RTOS
SDK (main pushed 2026-10-15). The older `kendryte/k230_sdk` (Linux+RT-Smart big/little)
was last pushed in 2025-09 and is legacy. `kendryte/canmv_k230` is now just the MicroPython
component, pulled in as `src/canmv` by this same manifest.

Linux for the same boards lives in `32_02_yocto` (`layers/meta-k230`, based on
`k230_linux_sdk`, machines `k230-canmv-01studio`, `k230d-canmv-bpi-zero`). That is a
different OS on the same hardware. Nothing here touches it, and each SD card holds one OS.

## Layout

| Path | What |
|---|---|
| `sdk -> $MCU_OUT_BASE/k230/sdk` | repo tree (≈5 GB src); build output in `sdk/output/<defconfig>/` |
| `toolchain -> $MCU_OUT_BASE/k230/toolchain` | `SDK_TOOLCHAIN_DIR` (musl RT-Smart + Xuantie glibc), 3.1 GB |
| `bin/repo` | git-repo launcher (canmv-k230/git-repo @c44ad09, repo 2.45) |
| `.venv/` | uv venv on k230-local CPython 3.10.22 (`host/python`): scons 3.1.2, pycryptodome, gmssl, jinja2, jsonschema, cmake 3.31.6, pyserial, k230-flash 1.4.0 (`scripts/requirements.txt`) |
| `host/` | from-source host tools: m4 1.4.19, bison 3.8.2, flex 2.6.4, mtools 4.0.48, dosfstools 4.2, rsync 3.4.1, `host/coreutils` (GNU coreutils 9.7, last on PATH) |
| `scripts/build-hosttools.sh` | rebuilds all of `host/` and `.venv` (pinned URLs + sha256; idempotent) |
| `env.sh`, `sdk.sh` | shell env (bash/zsh; sources `../host/env.sh` first) and the `../mcu` plugin |
| `docker/`, `k230-docker.sh` | optional legacy route: Ubuntu 22.04 image `k230-rtos-build:22.04`, runs as your uid (`K230_DOCKER=1`) |
| `manifest-frozen-*.xml` | `repo manifest -r` snapshot = exact commits of every project |

**State 2026-10-24 (evening):** re-fetched into the new location after `<build disk>/build/mcu/k230` was deleted
(sdk 4.9 GB, toolchain 3.1 GB, revisions = `manifest-frozen-20261023.xml`); `mcu doctor k230` and `mcu selfcheck k230`
pass. To rebuild from scratch again (≈5 GB sdk + 3.1 GB toolchain, network):

```bash
K=$MCU_OUT_BASE/k230; T=$BOX/30_39_tools/32_embedded/32_01_mcu/toolchain/k230
mkdir -p $K/sdk $K/toolchain && ln -sfn $K/sdk $T/sdk && ln -sfn $K/toolchain $T/toolchain
source $T/env.sh && cd $K/sdk
repo init -u https://github.com/canmv-k230/manifest -m default.xml
cp $T/manifest-frozen-20261023.xml .repo/manifests/ && repo init -m manifest-frozen-20261023.xml
repo sync -j8 -c --no-clone-bundle && make dl_toolchain     # toolchain lands in $K/toolchain (SDK_TOOLCHAIN_DIR)
$BOX/30_39_tools/32_embedded/32_01_mcu/tool/mcu doctor k230
```

`mcu build` hard-links build products into `$MCU_OUT_BASE/k230/<target>/<example>/`.

## Versions (pinned)

| Item | Version |
|---|---|
| manifest | canmv-k230/manifest `default.xml` @ 70e0709af41b (2026-08-27) |
| k230_rtos_sdk | main @ effb2f82e822 (2026-10-15, `v0.6-94`). The newest tag, v0.8 / release/canmv-v1.8, lacks `k230d_rtos_bpi_zero` |
| rtsmart / mpp / libs / canmv | 8b901a8496bb / 6e8da8e1a893 / ace49c5bc05a / 27cd7bbf0c9d (all: `manifest-frozen-20261023.xml`) |
| RT-Smart toolchain | `riscv64-unknown-linux-musl-rv64imafdcv-lp64d-20230420.tar.bz2` (GCC 12.0.1), sha256 8e09965b…ee08a |
| Xuantie glibc toolchain | `Xuantie-900-gcc-linux-5.10.4-glibc-x86_64-V2.6.0.tar.bz2` (GCC 10.2.0), sha256 17decb02…1f41a |
| Toolchain source | https://kendryte-download.canaan-creative.com/k230/toolchain/ |
| Host compiler | shared `../host/buildtools` (Yocto buildtools 6.0.4, gcc 15.3.0, make 4.4.1, git 2.53) |

## Owned boards → defconfig

| Board | RT-Smart only | CanMV MicroPython |
|---|---|---|
| Banana Pi BPI-CanMV-K230D-Zero (K230D, 128 MB in-package) | `k230d_rtos_bpi_zero_defconfig` | `k230d_canmv_bpi_zero_defconfig` |
| "Jia Nan" K230 CanMV 1 GB (01Studio-style) | `k230_rtos_01studio_defconfig` | `k230_canmv_01studio_defconfig` |

If the Jia Nan board does not boot the 01Studio image, try the generic Canaan
`k230_canmv_v3p0` / `k230_rtos_v3p0` (CanMV-K230 v3) or `*_lckfb`. `mcu targets k230` lists all.

## Build (native, self-contained)

```bash
cd $BOX/30_39_tools/32_embedded/32_01_mcu/tool
./mcu doctor k230                              # every tool must resolve inside mcu/
./mcu build k230 k230d_rtos_bpi_zero           # SD image
./mcu build k230 k230_rtos_01studio hello      # image + helloworld app
./mcu build k230 k230d_rtos_bpi_zero app       # only rebuild the hello ELF (seconds)
./mcu selfcheck k230                           # strict PATH (OS base utils only) build of SDK_SELFCHECK
```

By hand:

```bash
source $BOX/30_39_tools/32_embedded/32_01_mcu/toolchain/k230/env.sh
cd $K230_SDK
make k230d_rtos_bpi_zero_defconfig
make NCPUS=16 PARALLEL=16                      # whole image
```

There is one `.config` at the tree root, so builds are serialized: `sdk_build` holds a
`flock` on `sdk/.mcu-build.lock`. Each board has its own `output/<defconfig>/`. Measured
strict-mode native builds from an empty output dir: about 90 s for RT-Smart, about 75 s for
CanMV, with up to 16 jobs. `make menuconfig` needs ncurses headers, so run it with
`./k230-docker.sh make menuconfig` (or edit `.config` / `configs/*_defconfig`).

### Hello world (RT-Smart user-space ELF)

The source is `sdk/src/applications/helloworld/main.c`, enabled by
`CONFIG_APP_ENABLE_HELLOWORLD=y` (the `hello`/`app` examples set it). The result is a static
RISC-V ELF at `sdk/output/<defconfig>/images/sdcard/app/helloworld` (33 832 bytes),
installed in the image at `/sdcard/app/helloworld`. On the board's msh, run
`/sdcard/app/helloworld`. For a new app, add a dir with `Kconfig` + `Makefile` under
`src/applications/` (copy helloworld).

## Output images (built 2026-10-24, native strict mode)

| Target | Image (`$MCU_OUT_BASE/k230/…`) | Size |
|---|---|---|
| k230d_rtos_bpi_zero | `k230d_rtos_bpi_zero/hello/RtSmart-K230D_Zero_rtsmart_local_nncase_v2.11.0.img` (+ `helloworld`) | 1 220 542 464 B (gz 12.7 MB) |
| k230_rtos_01studio | `k230_rtos_01studio/hello/RtSmart-K230_01Studio_rtsmart_local_nncase_v2.11.0.img` (+ `helloworld`) | 1 220 542 464 B (gz 20.2 MB) |
| k230_canmv_01studio | `k230_canmv_01studio/image/CanMV_K230_01Studio_micropython_local_nncase_v2.11.0.img` | 671 088 640 B (gz 342 MB) |
| k230d_canmv_bpi_zero | `k230d_canmv_bpi_zero/image/CanMV_K230D_Zero_micropython_local_nncase_v2.11.0.img` | 671 088 640 B (gz 334 MB) |

Each directory holds `*.img` (raw SD card image), `*.img.gz` (compressed) and
`*_ota.kdimg` (partition image for k230-flash).

## Flash

- **SD card, Linux:** `./mcu flash k230 <target> [image|hello] /dev/sdX`. It refuses unless
  you name the device explicitly, the device is a whole disk with `RM=1`, and nothing on it is
  mounted. It then asks for `YES` (or takes `MCU_YES=1`). Without write permission on the
  device it prints the command instead:
  `dd if=<img> of=/dev/sdX bs=4M conv=fsync status=progress`.
- **balenaEtcher / Rufus / Win32DiskImager:** write the `.img` or `.img.gz` directly.
- **USB (no card reader, or eMMC/SPI flash):** hold BOOT and plug in USB, then run
  `k230-flash -l` and `k230-flash -m SDCARD|EMMC <…_ota.kdimg>`. This is the official
  `kendryte/k230_flash_py`; the GUI is `kendryte/k230_burning_tool`.
- **Serial console:** `./mcu monitor k230 /dev/ttyACM0 115200` (BPI Zero / CanMV USB-UART).

## Update

`./mcu update k230` reports the newest manifests and tags. To switch, run
`MCU_YES=1 ./mcu update k230 canmv_v1.8` (or `default`, `rtos_v0.8`). This re-inits repo on
that manifest, syncs, writes a new frozen manifest and appends a row to the versions log below.

## What still comes from the OS (and why)

- **Kernel, glibc, `/etc/ssl` CA certificates**, and the base utilities in `../host/sysbin.list`
  (bash, coreutils subset, sed, grep, gawk, find, tar, gzip, xz, bzip2, file, flock…). These
  are allowed by the self-contained rule.
- **Flash only:** `lsblk` and `dd` (util-linux/coreutils) in `sdk_flash`, plus write access to
  the block device. The tool never uses sudo.
- **Optional:** docker (legacy `K230_DOCKER=1` route and `menuconfig`).
- Nothing else. `./mcu doctor k230` fails if a build tool resolves outside `mcu/`, and
  `./mcu libcheck k230` reports 0 binaries that need OS libraries.
- The SDK's own Ubuntu instructions also list autoconf, automake, pkg-config, libconfuse,
  libssl-dev, libyaml-dev and gawk. Tracing the build shows they are unused: genimage is the
  Python `tools/genimage_py`, and U-Boot ships prebuilt.
- One known harmless warning: ffmpeg's Makefile calls `tput` (`Error 127 (ignored)`).

## Gotchas fixed here

- The SDK makefiles ignore sub-make failures. A missing tool (`patch` for the lvgl patches,
  `jsonschema`/`jinja2` for the mbedtls PSA generator) showed up later as
  `cJSON.h: No such file` in `examples/integrated_poc/smart_ipc`.
- `src/rtsmart/libs/mk/toolchain_riscv64_musl.mk` does
  `export PATH:="$(CROSS_COMPILE_DIR):$(PATH)"` with literal quotes, which corrupts the last
  PATH entry. `env.sh` therefore ends PATH with a dummy `host/empty`.
- GNU make's `$(shell pwd)` execs `pwd`, which is not in sysbin, so `host/coreutils` fills
  that gap.
- scons 3.1.2 (pinned by the SDK) does not run on Python ≥ 3.12. That is why this SDK uses a
  k230-local uv-managed CPython 3.10 instead of the shared 3.12, plus `SCONS_LIB_DIR`.
- `gen_image.sh` calls `repo info`, so `bin/repo` is on PATH.
- `host/` bison/flex were configured under the pre-reorg prefix `Box/Tools/Embedded/mcu/k230/host`, which is compiled in
  (bison: `m4sugar.m4: cannot open`). `env.sh` exports `BISON_PKGDATADIR` and `M4` to the current tree; a
  `scripts/build-hosttools.sh` rebuild would also fix it. The mtools binaries reference an old optional `etc/mtools.conf`, which is harmless.
- `sdk_doctor` also accepts tools under the physical `32_01_mcu` root, because the buildtools environment-setup exports physical
  `toolchain/host/...` paths while `MCU_ROOT` is the logical `tool/`.
- After a failed first build, `mcu selfcheck k230` (`app`) sees the partial `output/<defconfig>` and skips the full build
  (link fails with `-lsrtp2`/`-lusrsctp`); run `mcu build k230 k230d_rtos_bpi_zero hello` once first.

## Versions log

| Date | Manifest | SDK | Frozen manifest |
|---|---|---|---|
| 2026-10-23 | default.xml | k230_rtos_sdk effb2f82e822 | manifest-frozen-20261023.xml |
