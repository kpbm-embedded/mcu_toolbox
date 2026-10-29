# source this: AVR/CH552 environment (arduino-cli with all dirs inside mcu/avr; arduino-cli inherits this env)
if [ -n "${BASH_SOURCE:-}" ]; then _a="${BASH_SOURCE[0]}"; else _a="${(%):-%x}"; fi
_A="$(cd "$(dirname "$_a")" && pwd)"; unset _a
. "$_A/../host/env.sh"
export AVR_DIR="$_A" ARDUINO_CONFIG_FILE="$_A/cfg/arduino-cli.yaml"
export ARDUINO_DIRECTORIES_DATA="$_A/data" ARDUINO_DIRECTORIES_DOWNLOADS="$_A/downloads" ARDUINO_DIRECTORIES_USER="$_A/user"
export ARDUINO_BOARD_MANAGER_ADDITIONAL_URLS="https://raw.githubusercontent.com/DeqingSun/ch55xduino/ch55xduino/package_ch55xduino_mcs51_index.json file://$_A/cfg/package_drazzy.com_index.json"
export PATH="$_A/bin:$PATH"
_g=$(ls -d "$_A"/data/packages/arduino/tools/avr-gcc/*/bin 2>/dev/null | head -1); [ -n "$_g" ] && export PATH="$_g:$PATH"
_d=$(ls -d "$_A"/data/packages/arduino/tools/avrdude/*/bin 2>/dev/null | tail -1); [ -n "$_d" ] && export PATH="$_d:$PATH"
unset _A _g _d
