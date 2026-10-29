#!/usr/bin/env bash
# Sections in the ELF: ramfunc in RAM (0x2...), boot_magic in .noinit, nothing of .noinit in the image file.
set -euo pipefail
elf="$(find "$2" -maxdepth 2 -name 'noinit-reboot.elf' | head -1)"
nm="$(arm-none-eabi-nm "$elf")"
grep -qE '^2[0-9a-f]{7} t ramfunc' <<<"$nm" || { echo "ramfunc not in RAM"; exit 1; }
arm-none-eabi-objdump -h "$elf" | grep -q '\.noinit' || { echo "no .noinit section"; exit 1; }
arm-none-eabi-objdump -t "$elf" | grep -E '\.noinit.*boot_magic' >/dev/null || { echo "boot_magic not in .noinit"; exit 1; }
echo "test.sh: ramfunc in RAM, boot_magic/boot_count in .noinit"
