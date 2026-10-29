# ESP-IDF
| item | version |
|---|---|
| ESP-IDF | v6.1 (shallow clone + submodules, `esp-idf/esp-idf`) |
| Python | host-layer python 3.12.15 (../host), idf venv in tools/python_env; cmake/ninja from ../host |
| Toolchains | xtensa/riscv32-esp-elf gcc esp-15.2.0_20251204, in `tools/` (IDF_TOOLS_PATH, `~/.espressif` unused) |
Targets installed: esp32, esp32s3, esp32c3, esp32c6, esp32p4.

Use: `source env.sh` then `idf.py -B $MCU_OUT_BASE/esp-idf/<t>/blink -DIDF_TARGET=<t> -C esp-idf/examples/get-started/blink build`
or `../mcu build esp-idf <target> [example]`; flash `../mcu flash esp-idf esp32s3 get-started/blink /dev/ttyACM0`.
Re-install tools: `./install.sh`. Update: `MCU_YES=1 ../mcu update esp-idf`.

| Board | target | USB |
|---|---|---|
| ESP32-S3 devkit N16R8 / S3-Zero / HMI | esp32s3 | native USB (hold BOOT + reset for download) |
| ESP32-C3 SuperMini / Zero | esp32c3 | native USB-JTAG/CDC |
| ESP32 WROOM-32 CH340C | esp32 | CH340 /dev/ttyUSB0 |
| WT9932P4-TINY | esp32p4 (+ esp32c6 companion) | |
Serial access needs group `uucp`/`dialout`. udev (needs root): see `../tools` or install `https://github.com/espressif/openocd-esp32/blob/master/contrib/60-openocd.rules`.
Built (blink.bin): esp32 130768, esp32s3 187872, esp32c3 151296, esp32c6 157392, esp32p4 216336 bytes.

udev: `udev/60-openocd.rules` (root).
