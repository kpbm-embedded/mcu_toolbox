# Raspberry Pi Pico
| item | version |
|---|---|
| pico-sdk | 2.3.1 (+submodules) |
| pico-examples | sdk-2.3.1 |
| picotool | 2.3.1 built from source with static libusb 1.0.30 (`libusb/`), ldd: only libc/libm (`src-picotool`, flat install in `picotool/`, symlink `bin/picotool`) |
| ARM GCC | Arm GNU Toolchain 15.2.rel1 shared at ../tools/toolchains/arm-gnu-toolchain |
| RISC-V GCC | pico-sdk-tools v2.3.1-0 `riscv-toolchain-16` (riscv32-pico-elf 16.1.0) in `toolchain/riscv` |
Use: `../mcu build pico <target> [blink|hello_usb|<path>]`, `../mcu flash pico <target> [example]` (picotool load -x, or copy .uf2 to the BOOTSEL drive). `source env.sh`, `./build.sh <board> [platform]` also work.

| Board | target | PICO_BOARD / PLATFORM |
|---|---|---|
| Pico | pico | pico |
| RP2040-Zero / SuperMini / Core-A | zero | waveshare_rp2040_zero |
| Pico 2 (RP2350A) | pico2 | pico2 |
| Pico 2 RISC-V (Hazard3) | pico2-riscv | pico2 + rp2350-riscv |
| RP2350-Zero | rp2350-zero (rp2350-zero-riscv) | waveshare_rp2350_zero |
RP2350B tiny / Tenstar: use PICO_BOARD=pico2 or a custom header. USB access without root needs a udev rule (root): `SUBSYSTEM=="usb", ATTRS{idVendor}=="2e8a", MODE="0666"`.
Outputs in $MCU_OUT_BASE/pico/<target>/.

udev: `udev/99-pico.rules` (root). cmake/ninja/python come from ../host.
