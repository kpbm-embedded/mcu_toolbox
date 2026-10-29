# shellcheck shell=bash
# bouffalo build adapter: same flow as avr, delegates to the per-project build.sh (which stages a copy; SDK trees
# untouched). Needs `source bouffalo/env.sh` (done by check.sh).
# shellcheck source=SCRIPTDIR/avr.sh
. "$TOOLS/build/avr.sh"
