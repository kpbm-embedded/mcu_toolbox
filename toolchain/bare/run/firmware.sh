#!/usr/bin/env bash
# firmware.sh - run a firmware ELF built with bare.cmake in the emulator of its chip and check the self-test result.
#
# usage: firmware.sh <app.elf> [timeout-seconds]       (inside bare/env.sh: qemu and Renode on PATH)
# The run method comes from <app>.run, written by bare_firmware(): "qemu-arm <machine>", "qemu-riscv virt",
# "renode <repl> <uart>" or "none". Console output goes to stdout and to <app>.console.log.
# Exit: 0 when the firmware printed "RESULT: PASS" (and qemu exited 0), 1 on FAIL/timeout, 2 usage, 3 no emulator.
set -uo pipefail
elf="${1:-}"
case "$elf" in -h | --help | "") sed -n '2,8p' "$0" | sed 's/^# \{0,1\}//'; [[ -n "$elf" ]]; exit $(( $? ? 2 : 0 )) ;; esac
[[ -f "$elf" ]] || { echo "firmware.sh: no such ELF: $elf" >&2; exit 2; }
timeout_s="${2:-${BARE_RUN_TIMEOUT:-30}}"
base="${elf%.elf}"
log="$base.console.log"
read -r method arg1 arg2 <"$base.run" 2>/dev/null || { echo "firmware.sh: $base.run missing (not built with bare_firmware?)" >&2; exit 2; }
elf_abs="$(cd -P "$(dirname "$elf")" && pwd -P)/$(basename "$elf")"
rc=0
case "$method" in
  qemu-arm)
    timeout --foreground -k 5 "$timeout_s" qemu-system-arm -M "$arg1" -nographic -monitor none -serial stdio \
      -semihosting-config enable=on,target=native -kernel "$elf_abs" </dev/null 2>&1 | tee "$log"
    rc=${PIPESTATUS[0]} ;;
  qemu-riscv)
    timeout --foreground -k 5 "$timeout_s" qemu-system-riscv32 -M "$arg1" -bios none -nographic -monitor none \
      -serial stdio -kernel "$elf_abs" </dev/null 2>&1 | tee "$log"
    rc=${PIPESTATUS[0]} ;;
  renode)
    repl="$arg1"
    repl="@$repl"   # absolute, or relative to the Renode install (platforms/...)
    out="$base.uart.txt"; : >"$out"
    # Renode runs headless for a fixed emulated time, the UART goes to a file; then it quits.
    timeout --foreground -k 10 $((timeout_s + 60)) renode --disable-gui --plain --console --hide-log -e "
mach create \"bare\"
machine LoadPlatformDescription $repl
sysbus LoadELF @$elf_abs
sysbus.$arg2 CreateFileBackend @$out true
emulation RunFor \"00:00:${BARE_RENODE_SECONDS:-02}\"
quit" </dev/null >"$base.renode.log" 2>&1
    rc=$?
    cat "$out" | tr -d '\r' | tee "$log" ;;
  none) echo "firmware.sh: no emulator for this chip (build-only; flash a board)"; exit 3 ;;
  *) echo "firmware.sh: unknown run method '$method' in $base.run" >&2; exit 2 ;;
esac
if grep -q '^RESULT: PASS' "$log" && [[ $rc -eq 0 ]]; then exit 0; fi
echo "firmware.sh: no RESULT: PASS (emulator exit $rc; log $log)" >&2
exit 1
