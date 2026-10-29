# k210 plugin (see ../PLUGIN.md). Sourced by bash.
SDK_DESC="Kendryte K210 (CM-1 CanMV, Sipeed M1n): standalone + FreeRTOS SDKs, kflash"
SDK_SELFCHECK="standalone hello_world"

K_TC="$SDK_DIR/toolchain/kendryte-toolchain/bin"
K_PIN='-DCMAKE_POLICY_VERSION_MINIMUM=3.5'

sdk_env() { source "$SDK_DIR/env.sh"; }

_k_sdkdir() { case "$1" in standalone) echo kendryte-standalone-sdk;; freertos) echo kendryte-freertos-sdk;; *) return 1;; esac; }
_k_board() { case "$1" in maix-nano|cm1) echo goE;; standalone|freertos) echo goE;; *) return 1;; esac; }
_k_kind() { case "$1" in maix-nano|cm1) echo "${K_KIND:-standalone}";; *) echo "$1";; esac; }

sdk_doctor() {
  local rc=0 t v
  for t in riscv64-unknown-elf-gcc cmake make git kflash; do
    if v=$(command -v "$t" >/dev/null && { "$t" --version 2>&1 || true; } | head -1) && command -v "$t" >/dev/null; then echo "ok      $t  $v"; else echo "MISSING $t"; rc=1; fi
  done
  local d; for d in kendryte-standalone-sdk kendryte-freertos-sdk kendryte-standalone-demo; do
    [[ -d "$SDK_DIR/$d/.git" ]] && echo "ok      $d" || { echo "MISSING $d"; rc=1; }; done
  "$SDK_DIR/.venv/bin/python" -c 'import serial' 2>/dev/null && echo "ok      pyserial" || { echo "MISSING pyserial"; rc=1; }
  return $rc
}

sdk_versions() {
  local d; for d in kendryte-standalone-sdk kendryte-freertos-sdk kendryte-standalone-demo; do
    echo "$d  $(git -C "$SDK_DIR/$d" describe --tags --always 2>/dev/null)  $(git -C "$SDK_DIR/$d" rev-parse HEAD)"; done
  echo "toolchain  $(riscv64-unknown-elf-gcc --version | head -1)"
  echo "kflash     $("$SDK_DIR/.venv/bin/python" -c 'import importlib.metadata as m;print(m.version("kflash"))')"
}

sdk_targets() {
  printf '%-12s %s\n' standalone "kendryte-standalone-sdk (bare metal)" freertos "kendryte-freertos-sdk (FreeRTOS)" \
    maix-nano "Sipeed M1n Maix Nano (flash alias, -B goE; K_KIND=freertos to select SDK)" cm1 "CM-1 K210 CanMV (flash alias, -B goE)"
}

sdk_examples() {
  echo "hello_world (both SDKs)"
  ls "$SDK_DIR/kendryte-standalone-demo" | grep -v -e '^README' -e '^LICENSE' -e '^hello_world$' -e '^cmake' | while read -r n; do
    [[ -d "$SDK_DIR/kendryte-standalone-demo/$n" ]] && echo "$n (standalone demo)"; done
  [[ -d "$SDK_DIR/examples/blink" ]] && echo blink
}

sdk_build() {
  local target="${1:?target}" ex="${2:-hello_world}"; shift $(( $# >= 2 ? 2 : $# ))
  [[ "${1:-}" == -- ]] && shift
  local kind sdkd; kind=$(_k_kind "$target"); sdkd=$(_k_sdkdir "$kind") || { echo "unknown target $target" >&2; return 1; }
  local src="$SDK_DIR/$sdkd"
  if [[ ! -d "$src/src/$ex" ]]; then
    if [[ "$kind" == standalone && -d "$SDK_DIR/kendryte-standalone-demo/$ex" ]]; then
      ln -s "$SDK_DIR/kendryte-standalone-demo/$ex" "$src/src/$ex"   # symlink demo into SDK tree
    else echo "example '$ex' not found for $kind" >&2; return 1; fi
  fi
  local b="$MCU_OUT/$target/$ex"; mkdir -p "$b"; cd "$b"
  cmake "$src" -DPROJ="$ex" -DTOOLCHAIN="$K_TC" $K_PIN -G "Unix Makefiles" "$@" >cmake.log 2>&1 || { tail -20 cmake.log; return 1; }
  make -j"$MCU_JOBS" >make.log 2>&1 || { tail -30 make.log; return 1; }
  ls -l "$b/$ex" "$b/$ex.bin"; riscv64-unknown-elf-size "$b/$ex"
}

sdk_flash() {
  local target="${1:?target}" ex="${2:-hello_world}" port="${3:-/dev/ttyUSB0}" bin
  bin="$MCU_OUT/$target/$ex/$ex.bin"
  [[ -f "$bin" ]] || { echo "no build: $bin (run mcu build k210 $target $ex)" >&2; return 1; }
  kflash -p "$port" -b 1500000 -B "$(_k_board "$target")" "$bin"
}

sdk_monitor() { python -m serial.tools.miniterm "${1:-/dev/ttyUSB0}" "${2:-115200}"; }

sdk_clean() {
  local target="${1:?target}" ex="${2:-hello_world}" b="$MCU_OUT/${1}/${2:-hello_world}"
  [[ -d "$b" ]] || { echo "nothing at $b"; return 0; }
  mkdir -p ~/.local/share/Trash/files; mv "$b" ~/.local/share/Trash/files/"k210-$target-$ex-$(date +%s)"; echo "moved $b to Trash"
}

sdk_prune_families() { :; }
_k_py() { "$SDK_DIR/.venv/bin/python" -c "$1"; }
_k_kflash_ver() { _k_py 'import importlib.metadata as m;print(m.version("kflash"))'; }
_k_do_pull() { local d="$1"; upd_git_rec "$d"; git -C "$SDK_DIR/$d" fetch -q --tags origin && git -C "$SDK_DIR/$d" merge -q --ff-only origin/HEAD; }
_k_do_tag() { local d="$1" t="$2"; upd_git_rec "$d"; git -C "$SDK_DIR/$d" checkout -q "$t"; }
_k_do_kflash() { upd_rec pip ".venv/bin/python" "kflash==$(_k_kflash_ver)"; uv pip install -q --python "$SDK_DIR/.venv/bin/python" -U kflash; }
# standalone-sdk is pinned to a release tag (detached HEAD): move to the newest tag. freertos-sdk and the demo repo follow
# their upstream branch. A version/tag argument pins standalone-sdk (or "<dir>=<ref>" any of the three repos).
sdk_update() {
  UPD_RC=0
  local want="${1:-}" d cur new tag tgt
  for d in kendryte-standalone-sdk kendryte-freertos-sdk kendryte-standalone-demo; do
    git -C "$SDK_DIR/$d" fetch -q --tags origin 2>&1 | tail -1
    cur="$(git -C "$SDK_DIR/$d" describe --tags --always)"
    tgt=kendryte-standalone-sdk; [[ $want == *=* ]] && tgt="${want%%=*}"
    if [[ -n $want && $d == "$tgt" ]]; then
      tag="${want#*=}"; upd_step "$d" "$cur" "$tag" _k_do_tag "$d" "$tag"; continue
    fi
    if git -C "$SDK_DIR/$d" symbolic-ref -q HEAD >/dev/null; then       # branch checkout: follow upstream HEAD
      new="$(git -C "$SDK_DIR/$d" describe --tags --always origin/HEAD)"
      if [[ "$(git -C "$SDK_DIR/$d" rev-parse HEAD)" == "$(git -C "$SDK_DIR/$d" rev-parse origin/HEAD)" ]]; then new="$cur"; fi
      upd_step "$d" "$cur" "$new" _k_do_pull "$d"
    else                                                                # pinned to a tag: newest release tag (case-insensitive version sort)
      new="$(upd_git_newest_tag "$SDK_DIR/$d" '^[vV][0-9]+(\.[0-9]+)*$')"
      upd_step "$d" "$cur" "${new:-$cur}" _k_do_tag "$d" "$new"
    fi
  done
  upd_step kflash "$(_k_kflash_ver)" "$(upd_pypi_latest kflash)" _k_do_kflash
  upd_dry_hint
  return "$UPD_RC"
}
