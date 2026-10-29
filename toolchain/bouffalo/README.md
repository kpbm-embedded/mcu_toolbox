# Bouffalo Lab
| item | version |
|---|---|
| bouffalo_sdk | v2.3.37 (shallow, `bouffalo_sdk`) |
| Toolchain | Xuantie GCC riscv64-unknown-elf v2.10.2 (gcc 10.4.0) from github.com/bouffalolab/toolchain release `default` (the toolchain_gcc_t-head_linux repo only says it moved there) in `toolchain/` |
| Python/cmake/ninja | from ../host (self-contained) |
| Flasher | SDK BLFlashCommand-ubuntu (via `make flash COMX=port`) + `bflb-mcu-tool` in `venv/` (python 3.12) |
Use: `../mcu build bouffalo <bl616|bl702> [example]` (copies the example to $MCU_OUT_BASE/bouffalo/<t>/<ex>/src and builds there), or `. env.sh; cd bouffalo_sdk/examples/helloworld; make CHIP=bl616 BOARD=bl616dk`.
Flash: hold BOOT, tap RESET (or plug in with BOOT held), `../mcu flash bouffalo bl616 helloworld /dev/ttyACM0`. M0s Dock/Ai-M62 via USB CDC, M0sense via USB/UART.

| Board | target | CHIP / BOARD |
|---|---|---|
| Sipeed M0s Dock, Ai-M62 | bl616 | bl616 / bl616dk |
| Sipeed M0sense, RV-Debugger Plus, BL702C | bl702 | bl702 / bl702dk |
| Sipeed M1s Dock / M1s module (BL808) | bl808 (M0 core), bl808-d0 (D0 core) | bl808 / bl808dk, CPU_ID=m0 or d0; uses `bouffalo_sdk_legacy` (branch master_legacy @811e8c1, SDK 2.2.0) |
M1s Dock flashes through its onboard BL702 USB-UART, or BOOT+UART; BLFlashCommand / `bflb-mcu-tool --chipname bl808`. Built: helloworld_bl808_m0.bin 72176 B, helloworld_bl808_d0.bin 42848 B.
udev rules: `udev/99-bouffalo.rules` (install as root).
