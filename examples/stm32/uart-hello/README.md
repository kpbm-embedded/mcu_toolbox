# stm32 / uart-hello

printf over USART1 plus a polled echo of received bytes.

**Status: builds; untested on hardware.**

Prints a greeting and an uptime line each second; every byte received is echoed back and toggles the LED.

| Board (target) | LED | User button | Console USART1 |
|---|---|---|---|
| blackpill-f411 (WeAct BlackPill F411CEU6) | PC13, active low | KEY PA0, active low (pull-up) | PA9 TX / PA10 RX |
| weact-g431 (WeAct G431CBT6 core) | PC6, active high | PC13, active high (pull-down) | PA9 TX / PA10 RX |
| mini-h743 (H743VIT6 min-system) | PE3, active high | PC13, active high (pull-down) | PA9 TX / PA10 RX |
| bluepill-f103 (F103C8T6) | PC13, active low | none | PA9 TX / PA10 RX |

Expected output:
```
hello from blackpill-f411, SYSCLK 100000000 Hz
uptime 1 s, n=0
```

## Build
```
. mcu/stm32/env.sh
cmake -S mcu/examples/stm32/uart-hello -B $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/stm32/uart-hello/blackpill-f411 -G Ninja -DBOARD=blackpill-f411
cmake --build $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/stm32/uart-hello/blackpill-f411 -j16        # -> uart-hello.{elf,bin,hex}
```
Flash with any probe (see `mcu/stm32/README.md`): `st-flash --reset write uart-hello.bin 0x08000000`.
Console: `python -m serial.tools.miniterm /dev/ttyUSB0 115200` on a USB-UART wired to PA9/PA10 (115200 8N1).

Targets: blackpill-f411, weact-g431, mini-h743, bluepill-f103 (select with `-DBOARD=`).

Pin and clock sources: the `board.cmake` of each board in `mcu/stm32/boards/<board>/` cites them. They were cross-checked against the
Zephyr board files in this workspace (`blackpill_f411ce.dts`, `weact_stm32g431_core.dts`, `mini_stm32h743.dts`, `stm32_min_dev` blue overlay);
the vendor schematics were not re-read. Clocks: F411 100 MHz (HSE 25), G431 144 MHz (HSE 8), H743 400 MHz (HSE 25), F103 72 MHz (HSE 8).
