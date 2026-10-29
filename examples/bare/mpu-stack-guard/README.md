# mpu-stack-guard

MPU (PMSAv7) guard region below the stack: an overflow write raises MemManage, MMFAR names the address.

Chips: mps2-an385 (`mcu targets bare`). qemu models the Cortex-M3 MPU (8 regions). The handler records MMFAR, skips the faulting store and returns.

## Build and run

```sh
templates/tools/check.sh --only mpu-stack-guard bare            # build + emulator run (+ test.sh) for every chip
cd tool && ./mcu build bare mps2-an385 mpu-stack-guard          # one chip
$MCU_BARE/run/firmware.sh <build>/mpu-stack-guard.elf           # run it (inside: ./mcu shell bare)
```

The program checks itself and prints `RESULT: PASS` (qemu exit status 0; Renode: the runner reads the UART).
