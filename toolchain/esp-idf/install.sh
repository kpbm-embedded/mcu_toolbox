#!/bin/bash
_R="$(cd "$(dirname "$0")" && pwd)"
. "$_R/../host/env.sh"
export IDF_TOOLS_PATH="$_R/tools"
cd "$_R/esp-idf" && ./install.sh esp32,esp32s3,esp32c3,esp32c6,esp32p4
