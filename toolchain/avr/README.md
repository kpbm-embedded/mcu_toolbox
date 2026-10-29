# AVR + CH552 (mcu/avr)

Boards: Arduino Nano (ATmega328P, CH340), Mega2560, ATtiny85/13A, CH552T. `../mcu {doctor|build|flash|monitor} avr ...` or `source env.sh`.

| Component | Version |
|---|---|
| arduino-cli | 1.5.1 (bin/) |
| arduino:avr core | 1.8.8 (avr-gcc 7.3.0-atmel3.6.1-arduino7, avrdude 8.0.0-arduino1 / 6.3.0-arduino18) |
| ATTinyCore | 1.5.2 |
| CH55xDuino (mcs51, SDCC build.13407_4) | 0.0.26 |

All state is inside this dir: `cfg/arduino-cli.yaml` (set `ARDUINO_CONFIG_FILE`, done by env.sh), `data/`, `downloads/`, `user/`.

## Build / flash
- `./mcu build avr nano|nano-old|mega|attiny85|ch552` -> `$MCU_OUT_BASE/avr/<target>/Blink/`
- Direct: `arduino-cli compile -b arduino:avr:nano:cpu=atmega328old sketches/Blink`
- Upload: `arduino-cli upload -b arduino:avr:nano:cpu=atmega328old -p /dev/ttyUSB0 --input-dir <out>` ; monitor `arduino-cli monitor -p /dev/ttyUSB0 -c baudrate=115200`
  (CH340 Nano clones usually need `nano-old`/57600). ATtiny85 needs an ISP programmer (use avrdude -c usbasp etc. or Tools>Burn); CH552 uses its USB bootloader (BOOT + plug).
- CH55xDuino is a plain-C core: sketches are `.ino` with C only; use sketches-ch552/Blink (copied from the core's example).
- Bare-metal: `make -C baremetal` (atmega328p, PB5) or `make -C baremetal MCU=attiny85 F_CPU=8000000`; uses bundled avr-gcc.
- ATtiny13A is not supported by ATTinyCore; add MicroCore (`https://mcudude.github.io/MicroCore/package_MCUdude_MicroCore_index.json`) or use the baremetal Makefile with MCU=attiny13a.

## Index workaround (blocker noted)
On 2026-10-23 the TLS certs of `drazzy.com` and `azduino.com` were expired. The ATTinyCore index was fetched once with `curl -k`,
kept as `cfg/package_drazzy.com_index.json` (original `.orig`) and referenced via a `file://` URL in the config. In that copy the
`micronucleus` tool dependency (Digispark only, hosted on azduino.com) was removed. Core archive comes from SpenceKonde.github.io (valid TLS) and
is verified against the index SHA-256 (index itself fetched over unverified TLS, cross-check before trusting). When certs are fixed, re-add the http URL and drop the local copy.

## udev
CH340 (1a86:7523) and CH552 bootloader (4348:55e0) need access: add the user to `uucp`/`dialout` or use the rules in ../wch/udev/99-wch.rules (needs root).

## Self-contained
env.sh sources `../host/env.sh`; arduino-cli, avr-gcc, avrdude, ATTinyCore and CH55xDuino (sdcc, vnproch55x) are all in `avr/`. vnproch55x and sdar had OS libusb/libudev/libz: they now carry an `$ORIGIN` RUNPATH into `../host/lib` (`host/patch-rpath.sh avr/data`, rerun after `core upgrade`). arduino-cli inherits env.sh PATH.

## Layout and updates
`arduino-cli -> arduino-cli-<ver>` (stable symlink; `bin/arduino-cli` links into it). `./mcu update avr` (MCU_YES=1) updates the binary (checksums file verified) and `arduino-cli core upgrade`; `./mcu rollback avr` flips back and reinstalls the recorded core versions.
