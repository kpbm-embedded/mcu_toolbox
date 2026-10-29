# systick-irq

SysTick at 1 kHz plus a software-pended PendSV: interrupt counting, WFI sleep and exception priorities.

Chips: mps2-an385, stm32f411, nrf52840, stm32f103 (`mcu targets bare`). Uses only the Cortex-M core peripherals (SysTick, SCB), so the same code runs on every Arm chip.

## Build and run

```sh
templates/tools/check.sh --only systick-irq bare            # build + emulator run (+ test.sh) for every chip
cd tool && ./mcu build bare mps2-an385 systick-irq          # one chip
$MCU_BARE/run/firmware.sh <build>/systick-irq.elf           # run it (inside: ./mcu shell bare)
```

The program checks itself and prints `RESULT: PASS` (qemu exit status 0; Renode: the runner reads the UART).
