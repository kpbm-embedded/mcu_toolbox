# stm32 / usb-cdc

USB CDC-ACM virtual COM port echo, ST Cube USB Device middleware.

**Status: builds; untested on hardware.**

Uses `Middlewares/ST/STM32_USB_Device_Library` (Core + Class/CDC) **from the Cube pack of the board's family** (read-only, compiled out of tree) plus a local
`usbd_conf.c` (HAL_PCD glue, per-family `#if`), `usbd_desc.c` and CDC callbacks in `main.c`. VID/PID 0483:5740 is ST's published VCP pair; use your own for a product.

| Target | USB peripheral | 48 MHz USB clock | Notes |
|---|---|---|---|
| blackpill-f411 | OTG_FS, AF10 | PLLQ: SYSCLK lowered to 96 MHz (HSE 25 /25 x384, P /4, Q /8) via `BOARD_USB48` | VBUS sensing off (PA9 is console TX) |
| bluepill-f103 | USB FS device (FSDEV, packet memory) | PLL 72 MHz / 1.5 | D+ pull-up is a resistor on the board (many BluePills fit 10k instead of 1.5k: host may not enumerate, known BluePill issue). PA12 is pulled low for 10 ms at start so the host re-enumerates. Older USB library in the F1 pack (16-bit sizes, no TransmitCplt) handled by `#if` |
| weact-g431 | USB FS device (FSDEV) | PLL Q output (VCO 288 MHz / 6) enabled + `RCC_USBCLKSOURCE_PLL` | internal D+ pull-up (switched by the HAL) |
| mini-h743 | OTG2_FS (`USB_OTG_FS`), AF10_OTG2_FS | PLL3 Q: HSE 25 /5 x96 /10 | USB voltage detector enabled; VBUS sensing off (B-session override) |

USB pins: PA11/PA12 on all four (fixed USB pins of the parts). Board wiring sources: Zephyr `weact_stm32g431_core.dts` (`usb_dm_pa11 usb_dp_pa12`), `mini_stm32h743.dts`
(`usb_otg_fs_dm_pa11 usb_otg_fs_dp_pa12`, AF10 per hal_stm32 pinctrl), `blackpill_f411ce.dts` (`usb_otg_fs_dm_pa11 usb_otg_fs_dp_pa12`), `stm32_min_dev.dts` (`usb_dm_pa11 usb_dp_pa12`, USB clock 72 / 1.5). BluePill D+ pull-up value: unverified.

Expected: a new `/dev/ttyACM*` appears; anything typed into it is echoed back; the USART1 console prints `usb rx total N bytes`.

## Build
```
. mcu/stm32/env.sh
cmake -S mcu/examples/stm32/usb-cdc -B $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/stm32/usb-cdc/blackpill-f411 -G Ninja -DBOARD=blackpill-f411
cmake --build $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/stm32/usb-cdc/blackpill-f411 -j16        # -> usb-cdc.{elf,bin,hex}
```
Other boards: `-DBOARD=<target>` with its own build dir, or `MCU_JOBS=4 bash mcu/templates/tools/check.sh --only usb-cdc stm32`.
Flash with any probe (see `mcu/stm32/README.md`): `st-flash --reset write usb-cdc.bin 0x08000000`.
Console: `python -m serial.tools.miniterm /dev/ttyUSB0 115200` on a USB-UART wired to PA9/PA10 (115200 8N1).

Targets: blackpill-f411, weact-g431, mini-h743, bluepill-f103 (select with `-DBOARD=`).

Pin and clock sources: the `board.cmake` of each board in `mcu/stm32/boards/<board>/` cites them. They were cross-checked against the
Zephyr board files in this workspace (`blackpill_f411ce.dts`, `weact_stm32g431_core.dts`, `mini_stm32h743.dts`, `stm32_min_dev` blue overlay);
the vendor schematics were not re-read. Clocks: F411 100 MHz (HSE 25), G431 144 MHz (HSE 8), H743 400 MHz (HSE 25), F103 72 MHz (HSE 8).
