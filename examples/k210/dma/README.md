# k210 / dma

DMAC (6-channel AXI DMA controller) copies 16 KiB from one buffer to another:
1. channel 0, polled with `dmac_wait_done()`, timed with `sysctl_get_time_us()`;
2. channel 1, completion signalled by the DMA interrupt (`dmac_irq_register()` → PLIC);
3. the same copy by `memcpy()` for comparison.

Buffers come from `iomem_malloc()`, i.e. the uncached 0x40000000 alias of SRAM. The DMAC is not cache coherent; when it is
given ordinary (cached, 0x80000000) buffers the SDK's `FIX_CACHE` path (`lib/drivers/dmac.c`) silently copies through an
uncached bounce buffer, which would make the timing meaningless.

Status: builds; untested on hardware.

## Pins
None. Output goes to the UARTHS console (the USB-serial/ISP port, 115200 8N1). `board.h` is included only for the board
define; its LED/key pins are unconfirmed for M1n/CM-1 (see ../blink/README.md).

Expected output:
```
dma: src=0x40...... dst=0x40......, 4096 words
dma: polled copy ok in <n> us
dma: irq copy ok
dma: cpu memcpy (uncached) took <m> us
```

## Build
```
mcu/templates/tools/check.sh --only dma k210        # builds both targets into $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/k210/dma/<target>/
# by hand:
source mcu/k210/env.sh
cmake -S . -B $K210_BUILD/dma -DSDK=$K210_HOME/kendryte-standalone-sdk -DTOOLCHAIN=$K210_TOOLCHAIN -DBOARD=m1n -DCMAKE_POLICY_VERSION_MINIMUM=3.5
make -C $K210_BUILD/dma -j16
```

## Flash / monitor
`kflash -p /dev/ttyUSB0 -b 1500000 -B goE <build>/dma.bin` then `python -m serial.tools.miniterm /dev/ttyUSB0 115200` (kflash board id `goE` is a guess for these boards, see mcu/k210/README.md).
