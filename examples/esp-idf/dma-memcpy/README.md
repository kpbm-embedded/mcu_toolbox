# dma-memcpy

Memory-to-memory DMA with the `esp_async_memcpy` driver (GDMA channel; AHB GDMA on S3/C3/C6, AHB/AXI on P4, chosen by `esp_async_memcpy_install`). Copies 32 KiB from a DMA-capable buffer, waits for the completion ISR callback (semaphore), checks the data and compares the time with a CPU `memcpy`.

Status: builds; untested on hardware.

Hardware: none (log only). Needs `SOC_ASYNC_MEMCPY_SUPPORTED` (IDF `soc_caps.h`): ESP32-S3, C3, C6, P4. Classic ESP32 has no GDMA/async-memcpy engine (DMA only inside SPI/I2S peripherals), so it is not a target.

## Build, flash, monitor
```
source mcu/esp-idf/env.sh
T=esp32c3        # one of: esp32s3, esp32c3, esp32c6, esp32p4
idf.py -B $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/esp-idf/dma-memcpy/$T -DSDKCONFIG=$BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/esp-idf/dma-memcpy/$T/sdkconfig -DIDF_TARGET=$T build
idf.py -B <same build dir> -p /dev/ttyACM0 flash monitor
```
Hold BOOT and reset for download mode on native-USB boards.

Expected: every 2 s `round N: DMA 32768 bytes in <us> us (match), CPU memcpy <us> us`.
