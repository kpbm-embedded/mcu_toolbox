# shellcheck shell=bash
# k230 build adapter: RT-Smart user-space app, built OUT OF TREE with the project's own Makefile, which includes the SDK's
# src/applications/mkenv.mk and libs/mk/*.mk (read only). The SDK keeps one global .config, so the target's defconfig is
# selected under the same flock that `mcu build k230` uses (serialises with other K230 builds). Needs `source k230/env.sh`;
# the target's SDK output (rtsmart libs) must exist: run `mcu build k230 <target>` once. No file in the SDK tree is edited
# (select-defconfig rewrites the SDK's own generated .config / output/<target>_defconfig state, as `mcu build` does).
tpl_build() { # srcdir target outdir name
  local src="$1" target="$2" out="$3" sdk; sdk="$(readlink -f "$K230_SDK")"
  [[ -d $sdk/output/${target}_defconfig/rtsmart/libs/rtsmart_hal/lib ]] || { echo "no SDK build for $target: run ./mcu build k230 $target first"; return 1; }
  mkdir -p "$out/build"
  (
    exec 9>"$sdk/.mcu-build.lock"
    flock -w 3600 9 || { echo "timed out waiting for another k230 build"; exit 1; }
    env -u CFLAGS -u CXXFLAGS -u LDFLAGS -u CPPFLAGS -u MAKEFLAGS -u PYTHONPATH -u PYTHONHOME make -C "$sdk" "${target}_defconfig" >"$out/defconfig.log" 2>&1 || { tail "$out/defconfig.log"; exit 1; }
    env -u CFLAGS -u CXXFLAGS -u LDFLAGS -u CPPFLAGS -u MAKEFLAGS make -C "$sdk" .autoconf >>"$out/defconfig.log" 2>&1 || { tail "$out/defconfig.log"; exit 1; }
    env -u CFLAGS -u CXXFLAGS -u LDFLAGS -u CPPFLAGS -u MAKEFLAGS make -C "$src" -j"$MCU_JOBS" K230_SDK="$sdk" OUT="$out/build"
  ) || return 1
  echo "$out/build/app.elf" > "$out/.artifact"
}
