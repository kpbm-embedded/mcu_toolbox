# k230 plugin (see ../PLUGIN.md). Sourced by bash from $SDK_DIR.
# SDK: kendryte/k230_rtos_sdk via canmv-k230/manifest (RT-Smart + MPP + CanMV MicroPython).
# Builds run natively with the self-contained toolset (env.sh); K230_DOCKER=1 uses docker/ instead.
SDK_DESC="Kendryte K230/K230D RT-Smart + CanMV MicroPython (k230_rtos_sdk, self-contained native build)"

K_SDK="$SDK_DIR/sdk"
K_DOCKER="$SDK_DIR/k230-docker.sh"
K_OWNED="k230d_rtos_bpi_zero k230d_canmv_bpi_zero k230_rtos_01studio k230_canmv_01studio"

SDK_SELFCHECK="k230d_rtos_bpi_zero app"   # helloworld ELF only (incremental on an existing build)

sdk_env() { source "$SDK_DIR/env.sh"; }

_k_board() {
  case "$1" in
    k230d_rtos_bpi_zero)  echo "OWNED Banana Pi BPI-CanMV-K230D-Zero (RT-Smart only)";;
    k230d_canmv_bpi_zero) echo "OWNED Banana Pi BPI-CanMV-K230D-Zero (CanMV MicroPython)";;
    k230_rtos_01studio)   echo "OWNED 'Jia Nan' / 01Studio CanMV-K230 1GB (RT-Smart only)";;
    k230_canmv_01studio)  echo "OWNED 'Jia Nan' / 01Studio CanMV-K230 1GB (CanMV MicroPython)";;
    *) echo "";;
  esac
}

sdk_doctor() {
  local rc=0 t p
  # MCU_ROOT is the logical tool/ dir (plugins are symlinks into ../toolchain); the shared buildtools
  # environment-setup exports physical toolchain/host/... paths, so also accept the physical 32_01_mcu root.
  local phys; phys="$(cd -P "$MCU_ROOT/.." && pwd -P)"
  for t in gcc g++ make git tar patch wget python3 scons cmake bison flex m4 mcopy mmd rsync; do
    p=$(command -v "$t" 2>/dev/null || true)
    if [[ -z "$p" ]]; then echo "MISSING $t"; rc=1; continue; fi
    case "$p" in "$MCU_ROOT"/*|"$phys"/*) ;; *) echo "OS!     $t  $p (not self-contained)"; rc=1; continue;; esac
    p="${p#"$MCU_ROOT"/}"; p="${p#"$phys"/}"
    printf 'ok      %-8s %s  (%s)\n' "$t" "$( { "$t" --version 2>&1 || true; } | head -1)" "$p"
  done
  for t in "$SDK_DIR/host/sbin/mkdosfs" "$SDK_DIR/bin/repo"; do
    [[ "$(command -v "$(basename "$t")")" == "$t" ]] && echo "ok      $(basename "$t")  ${t#$MCU_ROOT/}" || { echo "MISSING $t"; rc=1; }
  done
  for t in "$K230_MUSL_BIN/riscv64-unknown-linux-musl-gcc" "$K230_GLIBC_BIN/riscv64-unknown-linux-gnu-gcc"; do
    if [[ -x "$t" ]]; then echo "ok      $(basename "$t")  $("$t" --version | head -1)"; else echo "MISSING $t"; rc=1; fi
  done
  python3 -c 'import Crypto, gmssl, jinja2, jsonschema, serial' 2>/dev/null && echo "ok      python modules (Crypto gmssl jinja2 jsonschema serial)" || { echo "MISSING python modules (scripts/build-hosttools.sh)"; rc=1; }
  [[ -f "$K_SDK/Makefile" && -d "$K_SDK/.repo" ]] && echo "ok      sdk tree $(git -C "$K_SDK" rev-parse --short HEAD)" || { echo "MISSING sdk tree $K_SDK"; rc=1; }
  [[ -x "$SDK_DIR/.venv/bin/k230-flash" ]] && echo "ok      k230-flash (USB burning)" || { echo "MISSING k230-flash"; rc=1; }
  return $rc
}

sdk_versions() {
  echo "k230_rtos_sdk  $(git -C "$K_SDK" describe --tags --always 2>/dev/null)  $(git -C "$K_SDK" rev-parse HEAD)  ($(git -C "$K_SDK" log -1 --format=%cs))"
  echo "manifest       $(git -C "$K_SDK/.repo/manifests" rev-parse HEAD 2>/dev/null)  file=$(sed -n 's/.*name="\([^"]*\)".*/\1/p' "$K_SDK/.repo/manifest.xml" 2>/dev/null | head -1)"
  local p; for p in src/rtsmart/rtsmart src/rtsmart/mpp src/rtsmart/libs src/uboot/uboot src/opensbi/opensbi src/canmv; do
    printf '  %-22s %s\n' "$p" "$(git -C "$K_SDK/$p" rev-parse --short=12 HEAD 2>/dev/null)"; done
  echo "musl gcc       $("$K230_MUSL_BIN/riscv64-unknown-linux-musl-gcc" --version | head -1)"
  echo "glibc gcc      $("$K230_GLIBC_BIN/riscv64-unknown-linux-gnu-gcc" --version | head -1)"
  echo "host tools     $(bison --version | head -1), $(flex --version), $(mcopy --version | head -1), rsync $(rsync --version | head -1 | awk '{print $3}'), scons $(scons --version | sed -n 's/.*engine: v\([0-9.]*\).*/\1/p'), python $(python3 -V | cut -d' ' -f2), $(gcc --version | head -1)"
  echo "k230-flash     $("$SDK_DIR/.venv/bin/python" -c 'import importlib.metadata as m;print(m.version("k230-flash"))' 2>/dev/null)"
}

sdk_targets() {
  local t c
  for t in $K_OWNED; do printf '%-36s %s\n' "$t" "$(_k_board "$t")"; done
  for c in "$K_SDK"/configs/*_defconfig; do
    t=$(basename "$c" _defconfig)
    [[ " $K_OWNED " == *" $t "* ]] || printf '%-36s %s\n' "$t" "-"
  done
}

sdk_examples() {
  echo "image   full SD-card image (default)"
  echo "hello   image + src/applications/helloworld (RT-Smart user ELF, /sdcard/app/helloworld)"
  echo "app     helloworld ELF only (fast; full hello build first if the target was never built)"
}

# _k_make <args...> : native make with the self-contained toolset (env.sh PATH)
# (K230_DOCKER=1: run inside the Ubuntu build container instead; legacy fallback)
_k_make() {
  if [[ "${K230_DOCKER:-0}" == 1 ]]; then "$K_DOCKER" make NCPUS="$MCU_JOBS" PARALLEL="$MCU_JOBS" "$@"; return; fi
  env -u CFLAGS -u CXXFLAGS -u LDFLAGS -u CPPFLAGS -u MAKEFLAGS -u PYTHONPATH -u PYTHONHOME \
    make -C "$(readlink -f "$K_SDK")" NCPUS="$MCU_JOBS" PARALLEL="$MCU_JOBS" "$@"
}

# sdk_build <target> [image|hello|app]
#   image  full SD-card image (default)
#   hello  full image with src/applications/helloworld enabled
#   app    only (re)build the helloworld ELF; needs one earlier image/hello build of <target>
sdk_build() {
  local target="${1:?target}" ex="${2:-image}"; shift $(( $# >= 2 ? 2 : $# ))
  [[ "${1:-}" == -- ]] && shift
  target="${target%_defconfig}"
  [[ -f "$K_SDK/configs/${target}_defconfig" ]] || { echo "unknown target $target (mcu targets k230)" >&2; return 1; }
  case "$ex" in image|hello|app) ;; *) echo "unknown example $ex (image|hello|app)" >&2; return 1;; esac
  local b="$MCU_OUT/$target/$ex" o; o="$(readlink -f "$K_SDK")/output/${target}_defconfig"
  if [[ "$ex" == app && ! -d "$o/rtsmart" ]]; then
    echo "app: $target has no SDK build yet, doing a full 'hello' build first"; ex=hello; b="$MCU_OUT/$target/app"; fi
  mkdir -p "$b"
  echo "building $target/$ex (log: $b/build.log)"
  (
    # one .config per SDK tree: serialize all k230 builds (other shells, mcu selfcheck)
    exec 9>"$(readlink -f "$K_SDK")/.mcu-build.lock"
    flock -w 3600 9 || { echo "timed out waiting for another k230 build"; exit 1; }
    _k_make "${target}_defconfig" || exit 1
    if [[ "$ex" != image ]]; then
      sed -i 's/^# CONFIG_APP_ENABLE_HELLOWORLD is not set/CONFIG_APP_ENABLE_HELLOWORLD=y/' "$K_SDK/.config"
    fi
    if [[ "$ex" == app ]]; then
      _k_make .autoconf || exit 1                                        # syncconfig -> autoconf.h
      _k_make -C src/applications/helloworld all "$@" || exit 1
      echo "Build K230 done (app only)"
    else
      _k_make "$@"
    fi
  ) >"$b/build.log" 2>&1 || { tail -30 "$b/build.log"; return 1; }
  grep -q '^Build K230 done' "$b/build.log" || { tail -30 "$b/build.log"; return 1; }
  # hard-link the artifacts (same filesystem, no extra space)
  local f
  if [[ "$ex" != app ]]; then
    for f in "$o"/*.img "$o"/*.img.gz "$o"/*.kdimg "$o"/*.md5; do [[ -f "$f" ]] && ln -f "$f" "$b/"; done
  fi
  [[ "$ex" != image && -f "$o/images/sdcard/app/helloworld" ]] && ln -f "$o/images/sdcard/app/helloworld" "$b/helloworld"
  echo "artifacts in $b:"; ls -l "$b" | grep -v build.log
}

sdk_flash() {
  local target="${1:?target}" ex="${2:-image}" dev="${3:-}"
  target="${target%_defconfig}"
  local img; img=$(ls -t "$MCU_OUT/$target/$ex"/*.img 2>/dev/null | head -1)
  [[ -n "$img" ]] || { echo "no image in $MCU_OUT/$target/$ex (mcu build k230 $target $ex)" >&2; return 1; }
  if [[ -z "$dev" ]]; then
    echo "usage: mcu flash k230 $target $ex /dev/sdX   (SD card; the device is never guessed)" >&2
    echo "removable disks now:" >&2; lsblk -dno NAME,RM,SIZE,MODEL,TRAN | awk '$2==1{print "  /dev/"$0}' >&2
    echo "USB burning (no SD card): $SDK_DIR/.venv/bin/k230-flash -m SDCARD|EMMC <kdimg>, see README" >&2
    return 1
  fi
  [[ -b "$dev" ]] || { echo "$dev is not a block device" >&2; return 1; }
  [[ "$(lsblk -dno TYPE "$dev")" == disk ]] || { echo "$dev is not a whole disk" >&2; return 1; }
  [[ "$(lsblk -dno RM "$dev" | tr -d ' ')" == 1 ]] || { echo "refusing: $dev is not removable (RM!=1)" >&2; return 1; }
  if lsblk -nro MOUNTPOINTS "$dev" 2>/dev/null | grep -q .; then
    echo "refusing: $dev has mounted partitions:" >&2; lsblk -o NAME,MOUNTPOINTS "$dev" >&2; return 1; fi
  lsblk -o NAME,RM,SIZE,MODEL,TRAN "$dev"
  echo "write $(basename "$img") ($(du -h "$img" | cut -f1)) to $dev ?"
  if [[ "${MCU_YES:-0}" != 1 ]]; then
    local a; read -r -p "type YES to continue: " a; [[ "$a" == YES ]] || { echo aborted; return 1; }
  fi
  [[ -w "$dev" ]] || { echo "no write permission on $dev (no sudo here); run yourself: dd if='$img' of=$dev bs=4M conv=fsync status=progress" >&2; return 1; }
  dd if="$img" of="$dev" bs=4M conv=fsync status=progress && sync && echo "done: $dev"
}

sdk_monitor() { "$SDK_DIR/.venv/bin/python" -m serial.tools.miniterm "${1:-/dev/ttyACM0}" "${2:-115200}"; }

sdk_clean() {
  local target="${1:?target}" ex="${2:-image}" ts; target="${target%_defconfig}"; ts=$(date +%s)
  local b="$MCU_OUT/$target/$ex" o="$K_SDK/output/${target}_defconfig"
  mkdir -p ~/.local/share/Trash/files
  [[ -d "$b" ]] && mv "$b" ~/.local/share/Trash/files/"k230-$target-$ex-$ts" && echo "moved $b to Trash"
  [[ -d "$o" ]] && mv "$o" ~/.local/share/Trash/files/"k230-sdkout-$target-$ts" && echo "moved $o to Trash"
  return 0
}

# sdk_update [manifest]  manifest = default | canmv_v1.8 | rtos_v0.8 | ... (file in canmv-k230/manifest)
_k230_do() { local m="$1" fz
  fz="rollback-$(date +%Y%m%d-%H%M%S).xml"
  # pin the current tree as a manifest inside the manifests repo so rollback is "repo init -m <that>; repo sync"
  ( cd "$K_SDK" && repo manifest -r -o ".repo/manifests/$fz" ) || return 1
  upd_rec repo sdk "$fz"
  ( cd "$K_SDK" && repo init -m "$m.xml" </dev/null && repo sync -j8 -c --no-clone-bundle ) || return 1
  _k_make dl_toolchain || echo "toolchain download failed; see tools/toolchain_*.mk for the tarball names"
  fz="$SDK_DIR/manifest-frozen-$(date +%Y%m%d).xml"
  ( cd "$K_SDK" && repo manifest -r -o "$fz" )
  printf '| %s | %s | k230_rtos_sdk %s | %s |\n' "$(date +%F)" "$m.xml" "$(git -C "$K_SDK" rev-parse --short=12 HEAD)" "$(basename "$fz")" >> "$SDK_DIR/README.md"; }
sdk_update() {
  UPD_RC=0
  local m="${1:-default}" mdir="$K_SDK/.repo/manifests"; m="${m%.xml}"
  git -C "$mdir" fetch -q origin 2>&1 | tail -1
  local cur_h up_h
  cur_h="$(git -C "$K_SDK" rev-parse --short=12 HEAD)"; up_h="$(git ls-remote https://github.com/kendryte/k230_rtos_sdk refs/heads/main | cut -c1-12)"
  echo "  manifests upstream: $(git -C "$mdir" ls-tree --name-only origin/master | grep xml | tr '\n' ' ')"
  echo "  newest release tags: $(git ls-remote --tags https://github.com/kendryte/k230_rtos_sdk | grep -v '\^{}' | sed 's|.*refs/tags/||' | sort -V | tail -3 | tr '\n' ' ')  (pinned to main: v0.8 lacks the owned boards)"
  upd_step k230_rtos_sdk "$cur_h" "${up_h:-$cur_h}" _k230_do "$m"
  upd_dry_hint
  return "$UPD_RC"
}
