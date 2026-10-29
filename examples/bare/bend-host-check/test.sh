#!/usr/bin/env bash
# Run the Bend reference on the host (32_31_bend) and compare with the firmware's printed value.
set -euo pipefail
here="$(cd -P "$(dirname "$0")" && pwd -P)"
bb="${BOX_BEND:-$here/../../../../32_31_bend}/bend-box"
[ -x "$bb" ] || { echo "SKIP: needs 32_31_bend"; exit 3; }
want="$(grep -o 'walk(0, 8, 1000) = [0-9]*' "$(find "$2" -name '*.console.log' | head -1)" | awk '{print $NF}')"
got="$("$bb" run bend "$here/src/ringidx.bend" | tail -1 | tr -dc '0-9')"
echo "test.sh: firmware $want, Bend $got"
[ "$want" = "$got" ]
