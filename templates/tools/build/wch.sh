# shellcheck shell=bash
# wch build adapter: copies the project into <outdir>/build and runs make there (ch32fun writes its products next to the
# sources, so the copy keeps the SDK checkout and the example tree clean). Needs `source wch/env.sh`.
tpl_build() { # srcdir target outdir name
  local src="$1" target="$2" out="$3" b="$3/build"
  if [[ -d $b ]]; then mkdir -p "$HOME/.local/share/Trash/files"; mv "$b" "$HOME/.local/share/Trash/files/wchbuild-$$-$RANDOM"; fi
  mkdir -p "$b" && cp -a "$src"/. "$b"/ || return 1
  rm -f "$b/example.toml"
  make -C "$b" -j"$MCU_JOBS" TARGET_NAME="$target" all || return 1
  local a; a="$(first_file --exclude _ext "$b"/*.bin "$b"/*/*.bin)"; echo "$a" > "$out/.artifact"
}
