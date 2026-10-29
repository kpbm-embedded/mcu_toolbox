# timer-irq-stm32

STM32 general-purpose timer TIM2 update interrupt through the NVIC (register level), counted against SysTick time.

Chips: stm32f411, stm32f103 (`mcu targets bare`). TIM2 has the same registers and IRQ number (28) on the F1 and F4; Renode models it.

## Build and run

```sh
templates/tools/check.sh --only timer-irq-stm32 bare            # build + emulator run (+ test.sh) for every chip
cd tool && ./mcu build bare stm32f411 timer-irq-stm32          # one chip
$MCU_BARE/run/firmware.sh <build>/timer-irq-stm32.elf           # run it (inside: ./mcu shell bare)
```

The program checks itself and prints `RESULT: PASS` (qemu exit status 0; Renode: the runner reads the UART).
