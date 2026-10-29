# shellcheck shell=bash disable=SC2034
# (SC2034: MCU, WS, WS_MCU are used by the .bats files that load this)
# Shared setup for the mcu bats tests.
#   MCU        the mcu under test (MCU_UNDER_TEST, default ../mcu), run against the real workspace (read-only commands)
#   WS_MCU     a throw-away workspace in $BATS_TEST_TMPDIR holding only the fake fixture SDK

MCU="${MCU_UNDER_TEST:-$(cd "$BATS_TEST_DIRNAME/.." && pwd)/mcu}"
REAL_ROOT="$(dirname "$MCU")"

# Isolate outputs, Trash and colors for every test.
setup_isolation() {
  export MCU_OUT_BASE="$BATS_TEST_TMPDIR/out"
  export MCU_TRASH="$BATS_TEST_TMPDIR/trash"
  export MCU_COLOR=never
  unset MCU_YES MCU_DRY_RUN MCU_VERBOSE MCU_QUIET MCU_JOBS
}

# Build a workspace with the real dispatcher + lib + host and the fake SDK only.
setup_fake_workspace() {
  setup_isolation
  WS="$BATS_TEST_TMPDIR/ws"
  mkdir -p "$WS"
  cp "$MCU" "$WS/mcu"
  ln -s "$REAL_ROOT/lib" "$WS/lib"
  link_host_layer "$WS/host"
  cp -r "$REAL_ROOT/tests/fixtures/fake" "$WS/fake"
  WS_MCU="$WS/mcu"
}

# The host layer without its plugin (so the fake workspace has exactly one SDK).
link_host_layer() {
  local dest="$1" entry
  mkdir -p "$dest"
  for entry in "$REAL_ROOT/host"/*; do
    [[ "$(basename "$entry")" == sdk.sh ]] || ln -s "$entry" "$dest/$(basename "$entry")"
  done
}

# Number of entries in the test Trash.
trash_count() {
  find "$MCU_TRASH/files" -mindepth 1 -maxdepth 1 2>/dev/null | wc -l
}
