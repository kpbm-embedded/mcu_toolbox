#!/usr/bin/env bash
# The .uf2 has valid blocks: magic numbers, nRF52840 family, first target address 0x26000; vectors at 0x26000.
set -euo pipefail
uf2="$(find "$2" -maxdepth 2 -name '*.uf2' | head -1)"
python3 -I - "$uf2" <<'EOF'
import struct, sys
d = open(sys.argv[1], "rb").read()
assert len(d) % 512 == 0 and d, "size"
m0, m1, flags, addr, size, n, total, fam = struct.unpack("<8I", d[:32])
assert (m0, m1, fam, addr) == (0x0A324655, 0x9E5D5157, 0xADA52840, 0x26000), (hex(m0), hex(fam), hex(addr))
assert total == len(d) // 512
sp, reset = struct.unpack("<II", d[32:40])
assert 0x20006000 < sp <= 0x20040000 and 0x26000 < reset < 0xF4000, (hex(sp), hex(reset))
print(f"test.sh: {total} UF2 blocks, family nRF52840, base 0x26000, reset vector {reset:#x}")
EOF
