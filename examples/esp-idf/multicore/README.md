# multicore

Dual-core SMP FreeRTOS on ESP-IDF: a producer task pinned to core 0 and a consumer task pinned to core 1 (`xTaskCreatePinnedToCore`) exchange timestamped messages over a FreeRTOS queue (cross-core latency is logged); `esp_ipc_call_blocking()` runs a function on each core explicitly.

Status: builds; untested on hardware.

Hardware: none (log only). Dual-core chips only: ESP32, ESP32-S3, ESP32-P4 (`SOC_CPU_CORES_NUM` = 2 in IDF `components/soc/<chip>/include/soc/soc_caps.h`). ESP32-C3/C6 are single-core: the build stops with `#error`.

## Build, flash, monitor
```
source mcu/esp-idf/env.sh
T=esp32s3        # one of: esp32, esp32s3, esp32p4
idf.py -B $BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/esp-idf/multicore/$T -DSDKCONFIG=$BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/esp-idf/multicore/$T/sdkconfig -DIDF_TARGET=$T build
idf.py -B <same build dir> -p /dev/ttyACM0 flash monitor
```
Hold BOOT and reset for download mode on native-USB boards.
ESP32 WROOM-32 CH340C: console on UART0 via the CH340, port /dev/ttyUSB0 (auto-reset into download mode via DTR/RTS).

Expected: `esp_ipc_call_blocking(core 0) ran on core 0`, same for core 1, then once per second `core 1 got #N from core 0, latency <us> us`.
