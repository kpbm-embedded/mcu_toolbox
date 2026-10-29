# stm32 / with-tests

`minimal` plus a hardware-free `src/logic.c` (clamp, triangle wave, SOS pattern) compiled for the MCU **and** for host unit tests
(Unity, vendored in `tests/unity/`, MIT) in `tests/`. The LED {{LED_PIN}} flashes SOS on {{TARGET}}.

## Build firmware
```
. mcu/stm32/env.sh
cmake -S . -B build -G Ninja -DBOARD={{TARGET}} && cmake --build build -j16     # -> build/{{NAME}}.{elf,bin,hex}
```
## Run the host tests (host gcc from mcu/host, no board needed)
```
cmake --build build --target host-tests
# or:  cmake -G Ninja -S tests -B build-host && cmake --build build-host && ctest --test-dir build-host --output-on-failure
```
Put every piece of logic that does not touch the HAL in `src/logic.c` and test it in `tests/test_logic.c`; keep `main.c` thin.
Targets: blackpill-f411, weact-g431, mini-h743, bluepill-f103 (clock/pin sources are cited in `mcu/stm32/boards/<board>/board.cmake`).
