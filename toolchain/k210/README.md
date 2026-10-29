# Kendryte K210 (CM-1 CanMV, Sipeed M1n Maix Nano)

## Pinned versions
- Toolchain: kendryte-gnu-toolchain v8.2.0-20190409 (ubuntu-amd64 tar.xz, GCC 8.2.0, riscv64-unknown-elf-). Works on glibc 2.44 unpatched. Tarball kept in dl/.
- kendryte-standalone-sdk: tag V0.5.7 = 70f9fdf177e8a078695374bef69a3a5683ad7498 (checked out; master 02576ba is 6 commits newer)
- kendryte-freertos-sdk: master b9dd7678f4c6549b0458a8ba9d80198691a2504d (v0.2.0-190)
- kendryte-standalone-demo: e89c35465fadb5524c892d2a1c7a76dc76e219ed
- kflash 1.1.6 (pyelftools 0.33, pyserial 3.5) in .venv (uv, python 3.14)
- Firmware: firmware/canmv_k210_standard.bin from github.com/kendryte/canmv release v1.0.7
  (sha256 9cc15cba837113feed962b63dfbafba0e894d4b802a3a6e36c070690bfdecdb5). MaixPy (sipeed/MaixPy) releases are now MaixCam-only.

## Build (out-of-tree, outputs on <build disk>)
    source env.sh
    # standalone
    mkdir -p $K210_BUILD/sa-hello && cd $K210_BUILD/sa-hello
    cmake $K210_HOME/kendryte-standalone-sdk -DPROJ=hello_world -DTOOLCHAIN=$K210_TOOLCHAIN -DCMAKE_POLICY_VERSION_MINIMUM=3.5
    make -j16
    # FreeRTOS: same with kendryte-freertos-sdk, build dir rtos-hello
Only deviation: -DCMAKE_POLICY_VERSION_MINIMUM=3.5 (cmake 4 rejects the SDKs' old cmake_minimum_required). No source patches.
Ready bins: out/standalone_hello_world.bin, out/freertos_hello_world.bin. Other apps: kendryte-standalone-demo/ (copy into SDK src/<name>, -DPROJ=<name>).

## Flash / monitor
    kflash -p /dev/ttyUSB0 -b 1500000 -B goE out/standalone_hello_world.bin
-B: no M1n/CM-1 entry exists. Maix Nano (M1n) is a Maix-Go-like/Dock-class board: try -B goE (or dan for Dock-style FT2232/CH340 auto-reset); if reset fails use -B dan, or hold BOOT/ISP low and reset. Unverified on hardware.
Monitor: picocom -b 115200 /dev/ttyUSB0 (or: python -m serial.tools.miniterm /dev/ttyUSB0 115200). Hello world prints "Hello World" on UART (standalone SDK).
Add -t for built-in terminal in kflash.
CanMV MicroPython: kflash -p /dev/ttyUSB0 -b 1500000 -B goE firmware/canmv_k210_standard.bin

## Self-contained
env.sh sources `../host/env.sh` (cmake, make, git, uv, CPython). `.venv` is built from the managed CPython 3.12 in `host/python` (kflash 1.1.6, pyserial, pyelftools); the Kendryte riscv toolchain is private to `toolchain/`. Only glibc and `host/sysbin.list` utilities come from the OS. Check: `./mcu selfcheck k210`, `./mcu libcheck k210`.

## Updates
`./mcu update k210` covers standalone-sdk (newest release tag, case-insensitive version sort; pass a tag or `<repo>=<ref>` to pin), freertos-sdk and the demo repo (follow upstream branch, fast-forward) and kflash; `./mcu rollback k210` re-checks-out the recorded commits.
