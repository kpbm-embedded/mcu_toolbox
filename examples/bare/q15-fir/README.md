# q15-fir

Fixed-point Q15 FIR filter checked against a float reference on the target (soft float on M3/RV32, FPU on the F411).

Chips: mps2-an385, stm32f411, virt-rv32 (`mcu targets bare`). On the STM32F411 the float reference runs on the FPU (hard-float ABI; the startup enables CP10/CP11 in CPACR).

## Build and run

```sh
templates/tools/check.sh --only q15-fir bare            # build + emulator run (+ test.sh) for every chip
cd tool && ./mcu build bare mps2-an385 q15-fir          # one chip
$MCU_BARE/run/firmware.sh <build>/q15-fir.elf           # run it (inside: ./mcu shell bare)
```

The program checks itself and prints `RESULT: PASS` (qemu exit status 0; Renode: the runner reads the UART).
