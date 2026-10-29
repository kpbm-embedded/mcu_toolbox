# shellcheck shell=bash
SDK_DESC="AVR (Nano/Mega/ATtiny85) + CH552 via arduino-cli, ATTinyCore, CH55xDuino, bundled avr-gcc/avrdude"
SDK_SELFCHECK="nano Blink"
sdk_env() { source "$SDK_DIR/env.sh"; }
sdk_doctor() {
  local rc=0 c
  command -v arduino-cli >/dev/null && echo "ok      arduino-cli $(arduino-cli version | head -1)" || { echo "MISSING arduino-cli"; rc=1; }
  command -v avr-gcc >/dev/null && echo "ok      avr-gcc $(avr-gcc -dumpversion)" || { echo "MISSING avr-gcc"; rc=1; }
  command -v avrdude >/dev/null && echo "ok      avrdude $(avrdude -? 2>&1 | grep -io 'version [0-9.]*' | head -1)" || { echo "MISSING avrdude"; rc=1; }
  for c in arduino:avr ATTinyCore:avr CH55xDuino:mcs51; do
    arduino-cli core list 2>/dev/null | grep -q "^$c " && echo "ok      core $(arduino-cli core list | grep "^$c " | awk '{print $1,$2}')" || { echo "MISSING core $c"; rc=1; }
  done
  return $rc
}
sdk_versions() { echo "arduino-cli $(arduino-cli version | head -1)"; arduino-cli core list; echo "avr-gcc $(avr-gcc -dumpversion)"; }
sdk_targets() {
  cat <<'T'
nano      Arduino Nano ATmega328P (new bootloader, 115200)   arduino:avr:nano:cpu=atmega328
nano-old  Arduino Nano clone, old bootloader (57600; CH340)  arduino:avr:nano:cpu=atmega328old
mega      Arduino Mega 2560                                  arduino:avr:mega:cpu=atmega2560
attiny85  ATtiny85 8MHz internal (ISP upload)                ATTinyCore:avr:attinyx5:chip=85,clock=8internal
ch552     CH552T (CH55xDuino, USB bootloader)                CH55xDuino:mcs51:ch552
<fqbn>    any FQBN also accepted
(ATtiny13A: needs MicroCore, not installed; baremetal/Makefile works with MCU=attiny13a)
T
}
sdk_examples() { echo "Blink  (default; sketches/Blink, ch552 uses sketches-ch552/Blink)"; ls sketches sketches-ch552 2>/dev/null | grep -v : ; echo "<dir>  any sketch directory path"; }
_fqbn() { case "$1" in
  nano) echo arduino:avr:nano:cpu=atmega328;; nano-old) echo arduino:avr:nano:cpu=atmega328old;;
  mega) echo arduino:avr:mega:cpu=atmega2560;; attiny85) echo ATTinyCore:avr:attinyx5:chip=85,clock=8internal;;
  ch552) echo CH55xDuino:mcs51:ch552;; *:*:*) echo "$1";; *) echo "unknown target $1" >&2; return 1;; esac; }
_sketch() { local t="$1" e="$2"
  [[ -d $e ]] && { echo "$e"; return; }
  if [[ $t == ch552 || $t == CH55xDuino:* ]]; then echo "$SDK_DIR/sketches-ch552/$e"; else echo "$SDK_DIR/sketches/$e"; fi; }
sdk_build() {
  local t="${1:?target}" e="${2:-Blink}" f s out; f="$(_fqbn "$t")" || return 1; s="$(_sketch "$t" "$e")"
  [[ -d $s ]] || { echo "no sketch $s" >&2; return 1; }
  out="$MCU_OUT/$t/$e"; mkdir -p "$out"
  arduino-cli compile -j "$MCU_JOBS" -b "$f" --output-dir "$out" "$s" 2>&1 | sed 's/\x1b\[[0-9;]*m//g' | grep -E "Sketch uses|Global|rror" 
  ls -l "$out"/*.hex
}
sdk_flash() { # target [example] [port]
  local t="${1:?target}" e="${2:-Blink}" p="${3:-/dev/ttyUSB0}" f; f="$(_fqbn "$t")" || return 1
  [[ $t == ch552 ]] && p="${3:-}"
  arduino-cli upload -b "$f" ${p:+-p "$p"} --input-dir "$MCU_OUT/$t/$e"
}
sdk_monitor() { arduino-cli monitor -p "${1:-/dev/ttyUSB0}" -c "baudrate=${2:-115200}"; }
sdk_clean() {
  local d="$MCU_OUT/${1:?target}/${2:-Blink}"
  [[ -d $d ]] || { echo "nothing at $d"; return 0; }
  mkdir -p ~/.local/share/Trash/files; mv "$d" ~/.local/share/Trash/files/"avr-$1-${2:-Blink}-$(date +%s)"; echo "moved to Trash"
}
sdk_prune_families() { printf 'arduino-cli|arduino-cli-[0-9]*\n'; }
_a_do_cli() { local t="$1" v="${1#v}" info url dg sha a="arduino-cli_${1#v}_Linux_64bit.tar.gz"
  info="$(upd_gh_asset arduino/arduino-cli "$t" 'arduino-cli_.*_Linux_64bit\.tar\.gz')"; IFS=$'\t' read -r url dg sha <<<"$info"
  # arduino publishes a checksums file per release, not per asset
  local cs; cs="$(upd_curl -m 60 "https://github.com/arduino/arduino-cli/releases/download/$t/${v:+arduino-cli_${v}_checksums.txt}" 2>/dev/null | grep " $a\$" | cut -d' ' -f1)"
  upd_fetch "$url" "$SDK_DIR/dl/$a" "${cs:+hex:$cs}${cs:-${dg:+hex:$dg}}" || return 1
  if [[ ! -d $SDK_DIR/arduino-cli-$v ]]; then
    mkdir -p "$SDK_DIR/arduino-cli-$v.part" && tar -xf "$SDK_DIR/dl/$a" -C "$SDK_DIR/arduino-cli-$v.part" arduino-cli && mv -T "$SDK_DIR/arduino-cli-$v.part" "$SDK_DIR/arduino-cli-$v" || return 1
  fi
  upd_flip "$SDK_DIR/arduino-cli" "arduino-cli-$v"; }
_a_do_cores() { local c
  while read -r c v _; do upd_rec arduino x "$c@$v"; done < <(arduino-cli core list 2>/dev/null | tail -n +2)
  arduino-cli core upgrade 2>&1 | sed 's/\x1b\[[0-9;]*m//g' | tail -3; }
sdk_update() {
  UPD_RC=0
  local t cur; t="$(upd_gh_latest arduino/arduino-cli)"; cur="$(arduino-cli version | awk '{print $3}')"
  upd_step arduino-cli "$cur" "${t#v}" _a_do_cli "$t"
  arduino-cli core update-index 2>&1 | sed 's/\x1b\[[0-9;]*m//g' | grep -iE "fail|could not" || true
  local out; out="$(arduino-cli outdated 2>/dev/null | sed 's/\x1b\[[0-9;]*m//g')"
  local c inst new anyold=0
  while read -r c inst new _; do
    [[ $c == *:* && $inst =~ ^[0-9] && $new =~ ^[0-9] ]] || continue
    anyold=1; upd_report "core:$c" "$inst" "$new"
  done <<<"$out"
  if (( anyold )); then
    if upd_yes; then
      if _a_do_cores; then while read -r c inst new _; do [[ $c == *:* && $inst =~ ^[0-9] ]] && upd_log "core:$c" "$inst" "$new" updated; done <<<"$out"; else UPD_RC=1; fi
    fi
  else
    arduino-cli core list 2>/dev/null | tail -n +2 | while read -r c inst _; do [[ -n $c ]] && upd_report "core:$c" "$inst" "$inst"; done
  fi
  upd_dry_hint
  return "$UPD_RC"
}
