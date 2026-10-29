# WCH CH32V (mcu/wch)

Boards: CH32V003 (WeAct/EVT), CH32V203 (WeAct C8T6), CH32V307VCT6, CH32V305, CH32H417, CH32M007; probe WCH-LinkW.
Use via `../mcu {doctor|build|flash|clean|update} wch ...` or `source env.sh`.

| Component | Version | Where |
|---|---|---|
| xPack riscv-none-elf-gcc | 15.2.0-1 | toolchain/ |
| ch32fun (minichlink built from it, -O0 default build) | a800cbd (master 2026-10-23) | ch32fun/, bin/minichlink |
| wlink | 0.1.3 | bin/ |
| wchisp | 0.3.0 | bin/ |
| openwch/ch32v20x EVT | b375bbb | ch32v20x/ |
| openwch/ch32v307 EVT | 8b3699d | ch32v307/ (cloned, not yet built) |

Builds go to `$MCU_OUT_BASE/wch/<target>/<example>/`.

## Build
- `./mcu build wch ch32v003` (ch32fun examples/blink; rv32ec_zicsr/ilp32e handled by ch32fun.mk)
- `./mcu build wch ch32v203` (TARGET_MCU=CH32V203C8T6), `ch32v307`, `ch32h417` (examples_h41x/regular)
- `./mcu build wch ch32v203 evt-gpio`: official EVT GPIO_Toggle via `evt-v203-gpio/Makefile`.
  Flag tweaks vs MounRiver: `-march=rv32imac_zicsr_zifencei -mabi=ilp32 -msmall-data-limit=8 -mno-save-restore`
  (csr/fence.i are separate extensions in GCC 15), startup_ch32v20x_D6.S (C8T6 = D6 class),
  `--specs=nano.specs --specs=nosys.specs` (only link warnings about unimplemented syscalls).
- Manual ch32fun: `source env.sh; cd ch32fun/examples/blink; make build` (`make` alone also tries to flash).

## Flash
- `minichlink -w blink.bin flash -b` (WCH-Link/E/W, or ISP); `minichlink -T` terminal
- `wlink flash blink.hex` ; `wlink status`
- `wchisp flash blink.hex` (USB ISP bootloader: hold BOOT, plug in)
- via mcu: `./mcu flash wch ch32v003 blink [minichlink|wlink|wchisp]`
- WCH-LinkW needs RV mode (not ARM mode); `wlink` can switch with its mode commands/WCH-LinkUtility.

## udev (needs root, not installed)
`sudo cp udev/99-wch.rules /etc/udev/rules.d/ && sudo udevadm control --reload && sudo udevadm trigger`
(1a86:8010/8012 WCH-Link, 4348:55e0 and 1a86:55e0 ISP, 1209:b003 b003fun.)

## Notes
- openwch has repos `ch32h417` (EVT for CH32H417), `ch32v002_004_005_006_007` (no CH32M007 repo seen), `ch32v003`, `ch32v103`: not cloned.
- Flashing is untested (no hardware attached during setup).

## Self-contained
env.sh sources `../host/env.sh`. Private: xPack riscv gcc (`toolchain/`), `bin/{minichlink,wlink,wchisp}`. minichlink is rebuilt against static libusb 1.0.30 (`host/static`) and links `host/lib/libudev.so.1` (RUNPATH) for hidapi; wlink gets libudev via RUNPATH too (`host/patch-rpath.sh`). `ch32fun/examples_usb` test binaries are intentionally untouched. Only glibc and sysbin utilities come from the OS; udev rules need root (`udev/`).

## Layout and updates
Stable symlinks: `toolchain/xpack-riscv-none-elf-gcc`, `wlink`, `wchisp` (`bin/wlink`, `bin/wchisp` link into them). `./mcu update wch` updates the xPack gcc, wlink, wchisp (sha256 where published, else computed and logged in `.update-sha256`) and fast-forwards ch32fun/openwch (rebuilding minichlink); `./mcu rollback wch` undoes it.
