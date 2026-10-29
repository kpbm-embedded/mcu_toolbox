# wch / dual-core

CH32H417 has two RISC-V cores: the V3F boots, runs `main()` and starts the V5F with ch32fun's `StartV5F(main_V5F)`
(`FUNCONF_H41x_V5F_EN 1` in `funconfig.h`). Based on ch32fun `examples_h41x/dual_hello_world` (MIT). Both print their
hart id and stack pointer; the V5F then blinks the LED every 250 ms and counts, the V3F prints the V5F's counter (shared
SRAM variable) once per second.

Status: builds; untested on hardware.

## Pins
LED PE2 (used by every ch32fun `examples_h41x` program; **unconfirmed** for the owned CH32H417, there is no board file).
printf: ch32fun's `_write()` picks the UART by `mhartid`: V3F -> USART1 TX PA9 (AF7), V5F -> USART6 TX PA12 (AF6),
both 115200 8N1 (`ch32fun.c` SetupUART for CH32H41x). Connect two USB-UART adapters (or one, moving it between pins).

## Build
```
mcu/templates/tools/check.sh --only dual-core wch      # -> $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/wch/dual-core/ch32h417/build/
source mcu/wch/env.sh
B=$BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/mywork/dual-core; mkdir -p $B && cp -r . $B && make -C $B TARGET_NAME=ch32h417
```

## Flash / monitor
`minichlink -w main.bin flash -b` (WCH-LinkE). Monitor PA9 and PA12 at 115200.

## Expected
PA9: `Hello from V3F, hart 0, ...` then `V3F n: V5F ticks N` every second (N growing by ~4). PA12: `Hello from V5F, hart 1, ...`.
LED blinks at 2 Hz.
