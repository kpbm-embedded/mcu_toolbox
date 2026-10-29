#!/usr/bin/env bash
# Size budget: text (flash) and data+bss (RAM) of the ELF against budget.txt; prints the 5 largest symbols.
set -euo pipefail
target="$1" here="$(cd "$(dirname "$0")" && pwd)"
elf="$(find "$2" -maxdepth 2 -name 'size-budget.elf' | head -1)"
case "$target" in ch32v003) tool=riscv-none-elf ;; *) tool=arm-none-eabi ;; esac
read -r text data bss _ < <("$tool-size" "$elf" | tail -1)
read -r _ max_text max_ram < <(grep "^$target " "$here/budget.txt")
echo "test.sh: $target text $text / $max_text, data+bss $((data + bss)) / $max_ram"
"$tool-nm" --size-sort -S "$elf" | tail -5 | sed 's/^/  /'
[[ $text -le $max_text && $((data + bss)) -le $max_ram ]] || { echo "test.sh: over budget"; exit 1; }
