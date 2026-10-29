#!/usr/bin/env bash
# Run the Bend reference on the host (pinned Bend 2, `mcu setup install bend`) and compare with the firmware's printed value.
set -euo pipefail
here="$(cd -P "$(dirname "$0")" && pwd -P)"
root="$(cd -P "$here/../../.." && pwd -P)"
bend="$root/toolchain/tools/bend-2.0.39/bin/bend"
[ -x "$bend" ] || { echo "SKIP: needs Bend 2 (mcu setup install bend)"; exit 3; }
# Bend keeps its state in the ID's cache; no telemetry, never update/login/publish
h="$root/cache/bend-home"; mkdir -p "$h/tmp"
want="$(grep -o 'walk(0, 8, 1000) = [0-9]*' "$(find "$2" -name '*.console.log' | head -1)" | awk '{print $NF}')"
got="$(HOME="$h" TMPDIR="$h/tmp" BEND_LIB="$h/.bend/lib" BEND_NO_TELEMETRY=1 "$bend" "$here/src/ringidx.bend" | tail -1 | tr -dc '0-9')"
echo "test.sh: firmware $want, Bend $got"
[ -n "$want" ] && [ "$want" = "$got" ]
