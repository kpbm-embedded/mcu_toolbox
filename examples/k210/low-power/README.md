# k210 / low-power

The K210 has **no sleep, standby or deep-sleep mode** and no always-on wake-up domain (SDK `sysctl.h`: only PLL enable/disable,
clock enable/disable/select and IO bank voltage; no power-down register). What software can do, and what this example does:

1. switch off the KPU clock (`sysctl_clock_disable(SYSCTL_CLOCK_AI)`) and PLL1 that feeds it (`sysctl_pll_disable(SYSCTL_PLL1)`);
2. lower PLL0, hence the CPU and the AXI/APB buses, to 100 MHz (`sysctl_cpu_set_freq()`), then re-init the UARTHS divisor;
3. idle the core with `wfi` until an interrupt is pending: TIMER0 channel 0 every 2 s, or a falling edge on the BOOT key.

On each wake the green LED flashes and the console prints which source woke the core and how far `mcycle` advanced
(whether `mcycle` counts during WFI is not documented for the K210: the number tells you).
PLL2 (I2S, and the SPI3 sample clock) is left running.

Status: builds; untested on hardware. Current draw not measured.

## Pins
Source: Sipeed Maix family convention (Maix Bit V2.0 schematic: BOOT = IO16; MaixDuino pin table: IO12 = LED_B, IO13 = LED_R,
IO14 = LED_G, active low). **Not confirmed for the M1n Maix Nano or the CM-1 CanMV board**. Pins are `#define`s in `board.h`.

Expected output (115200 8N1 on the USB-serial port; boot clocks depend on the bootloader):
```
low-power boot: cpu 390000000 Hz, pll0 780000000, pll1 ..., apb0 ...
low-power reduced: cpu ~100000000 Hz, pll0 ~200000000, pll1 0, apb0 ...
wake: timer (timer 1, key 0), mcycle advanced ... during wfi
wake: key (timer 1, key 1), ...
```

## Build
```
mcu/templates/tools/check.sh --only low-power k210        # builds both targets into $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/k210/low-power/<target>/
# by hand:
source mcu/k210/env.sh
cmake -S . -B $K210_BUILD/low-power -DSDK=$K210_HOME/kendryte-standalone-sdk -DTOOLCHAIN=$K210_TOOLCHAIN -DBOARD=m1n -DCMAKE_POLICY_VERSION_MINIMUM=3.5
make -C $K210_BUILD/low-power -j16
```

## Flash / monitor
`kflash -p /dev/ttyUSB0 -b 1500000 -B goE <build>/low-power.bin` then `python -m serial.tools.miniterm /dev/ttyUSB0 115200` (kflash board id `goE` is a guess for these boards, see mcu/k210/README.md).
