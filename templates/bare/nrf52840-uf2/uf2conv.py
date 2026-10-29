#!/usr/bin/env python3
"""uf2conv.py <app.bin> <base-address> <out.uf2>: wrap a binary into UF2 blocks (256-byte payload) for nRF52840."""
import struct
import sys

MAGIC0, MAGIC1, MAGIC_END = 0x0A324655, 0x9E5D5157, 0x0AB16F30
FAMILY_NRF52840 = 0xADA52840


def main() -> int:
    if len(sys.argv) != 4:
        print(__doc__)
        return 2
    data = open(sys.argv[1], "rb").read()
    base = int(sys.argv[2], 0)
    blocks = [data[i:i + 256] for i in range(0, len(data), 256)]
    with open(sys.argv[3], "wb") as out:
        for n, chunk in enumerate(blocks):
            head = struct.pack("<8I", MAGIC0, MAGIC1, 0x2000, base + 256 * n, 256, n, len(blocks), FAMILY_NRF52840)
            out.write(head + chunk.ljust(476, b"\0") + struct.pack("<I", MAGIC_END))
    return 0


if __name__ == "__main__":
    sys.exit(main())
