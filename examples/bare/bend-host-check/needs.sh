#!/usr/bin/env bash
# Exit 3 (skip) when the optional pinned Bend 2 is not installed: `mcu setup install bend` (config/pins.tsv).
here="$(cd -P "$(dirname "$0")" && pwd -P)"
[ -x "$here/../../../toolchain/tools/bend-2.0.39/bin/bend" ] || { echo "SKIP: needs Bend 2 (mcu setup install bend)"; exit 3; }
