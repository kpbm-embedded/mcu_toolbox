# stm32 / dma-uart

USART1 echo with DMA: RX idle-line framed, TX via DMA.

**Status: builds; untested on hardware.**

RX uses `HAL_UARTEx_ReceiveToIdle_DMA` (half-transfer interrupt off), TX `HAL_UART_Transmit_DMA`. Each idle-framed chunk is copied and echoed; a chunk arriving while the previous echo is in flight is dropped.

| Target | RX DMA | TX DMA | Source of the mapping |
|---|---|---|---|
| blackpill-f411 | DMA2 Stream2 ch4 | DMA2 Stream7 ch4 | RM0383 DMA2 request table |
| bluepill-f103 | DMA1 Channel5 | DMA1 Channel4 | RM0008 DMA1 request table; same in the Cube F1 example `STM32F103RB-Nucleo/Examples/UART/UART_TwoBoards_ComDMA` (`USARTx_RX_DMA_CHANNEL`) |
| weact-g431 | DMA1 Channel1 | DMA1 Channel2 | any channel via DMAMUX: `DMA_REQUEST_USART1_RX/TX` |
| mini-h743 | DMA1 Stream0 | DMA1 Stream1 | any stream via DMAMUX: `DMA_REQUEST_USART1_RX/TX`; buffers are in AXI SRAM (link address 0x24000000, reachable by DMA1), D-cache is not enabled by the board layer |

Expected: bytes typed in the terminal come back, the LED toggles per chunk.

## Build
```
. mcu/stm32/env.sh
cmake -S mcu/examples/stm32/dma-uart -B $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/stm32/dma-uart/blackpill-f411 -G Ninja -DBOARD=blackpill-f411
cmake --build $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/stm32/dma-uart/blackpill-f411 -j16        # -> dma-uart.{elf,bin,hex}
```
Other boards: `-DBOARD=<target>` with its own build dir, or `MCU_JOBS=4 bash mcu/templates/tools/check.sh --only dma-uart stm32`.
Flash with any probe (see `mcu/stm32/README.md`): `st-flash --reset write dma-uart.bin 0x08000000`.
Console: `python -m serial.tools.miniterm /dev/ttyUSB0 115200` on a USB-UART wired to PA9/PA10 (115200 8N1).

Targets: blackpill-f411, weact-g431, mini-h743, bluepill-f103 (select with `-DBOARD=`).

Pin and clock sources: the `board.cmake` of each board in `mcu/stm32/boards/<board>/` cites them. They were cross-checked against the
Zephyr board files in this workspace (`blackpill_f411ce.dts`, `weact_stm32g431_core.dts`, `mini_stm32h743.dts`, `stm32_min_dev` blue overlay);
the vendor schematics were not re-read. Clocks: F411 100 MHz (HSE 25), G431 144 MHz (HSE 8), H743 400 MHz (HSE 25), F103 72 MHz (HSE 8).
