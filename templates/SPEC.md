# Templates and examples: format

Two kinds of ready code, both **out of tree** (never inside an SDK checkout, never modifying SDK files):

| | Path | Purpose |
|---|---|---|
| Template | `mcu/templates/<sdk>/<template>/` | Skeleton for a NEW project. Instantiated with placeholders filled in. |
| Example | `mcu/examples/<sdk>/<example>/` | Complete, buildable demo of one feature on owned boards. Read it, build it, copy from it. |

## Template layout

```
mcu/templates/<sdk>/<template>/
  template.toml     # metadata (below)
  README.md         # what the template gives you, how to build/flash, where to start editing
  ...files...       # build files + sources; placeholders {{NAME}}, {{TARGET}}, {{BOARD}}, {{LED_PIN}}, ... in file contents and file names
```

`template.toml`:

```toml
name        = "minimal"                 # dir name
sdk         = "zephyr"
description = "Blink the board LED and print over UART; the smallest real app"
targets     = ["blackpill_f411ce", "rpi_pico2", "promicro_nrf52840"]   # each must build (checked)
default_target = "blackpill_f411ce"
[vars]                                   # extra placeholders with per-target values (optional)
LED_PIN = { blackpill_f411ce = "PC13", rpi_pico2 = "25" }
```

Every SDK has at least `minimal`. Add `rtos` where the SDK has an RTOS (zephyr threads, freertos, esp-idf tasks, k210 freertos-sdk, k230 rt-smart app) and `with-tests` (minimal + `tests/` host unit tests with Unity, built by a host CMake/make target) where the build system allows it.

## Example layout

```
mcu/examples/<sdk>/<example>/
  example.toml      # metadata (below)
  README.md         # what it demonstrates, wiring/pins per board, build/flash/monitor commands, expected output
  ...sources and build files (ready to build, no placeholders)...
```

`example.toml`:

```toml
name        = "button-irq"
sdk         = "stm32"
description = "External interrupt on a button toggles the LED"
targets     = ["blackpill-f411"]          # built by check.sh
hardware    = "WeAct BlackPill F411: KEY button PA0, LED PC13 (active low)"
features    = ["gpio", "exti", "nvic"]
status      = "builds; untested on hardware"   # change only after a real board test
```

Example catalogue: per SDK, cover the owned boards with the common set where the hardware supports it:
`blink`, `uart-hello`, `button-irq`, `pwm-fade`, `i2c-scan`, `adc-read`, `timer`, `usb-cdc`, `rtos-tasks` (+ queue/mutex),
plus the SDK's signature feature (pico `pio-ws2812`, esp-idf `wifi-scan`/`ble-scan`/`led-strip`, zephyr `shell`, k230 `rt-smart-app`, bouffalo `wifi-scan` on BL616, ...).
Pins must match the owned boards in `mcu/BOARDS.md`; say the source of every pin assignment (board schematic/vendor doc/SDK board file) in the README.

## Tooling (`mcu/templates/tools/`)

- `instantiate.py <sdk> <template> <dest> --name N [--target T]` (run with host python `-I`): copies, substitutes placeholders, refuses to overwrite an existing dest.
- `check.sh [sdk ...]`: builds every example for every listed target and instantiates + builds every template for every listed target, using the SDK's own `env.sh` and build commands, output to `$BOX_OUT/30_39_tools/32_embedded/32_01_mcu/build/examples/<sdk>/<name>/<target>/`. Prints a table name/target/OK|FAIL/artifact size; exit 1 on any FAIL. Must pass under strict PATH (`PATH=mcu/host/sysbin`), like `mcu selfcheck`.
- `index.py` regenerates `mcu/examples/README.md` and `mcu/templates/README.md` (catalogue tables from the toml files).
