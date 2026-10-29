# source this: ESP-IDF v6.1, self-contained (host layer python 3.12, cmake, ninja)
_R="$(cd "$(dirname "${BASH_SOURCE[0]:-$0}")" && pwd -P)"  # physical path: export.sh resolves IDF_PATH physically; keep IDF_TOOLS_PATH/python env consistent
_RH="$_R/.."; . "$_RH/host/env.sh"
export IDF_TOOLS_PATH="$_R/tools"
export IDF_PATH="$_R/esp-idf"
. "$_R/esp-idf/export.sh" >/dev/null 2>&1
