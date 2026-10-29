#!/usr/bin/env python3
"""mkimage.py <app.bin> <app.img>: prepend a 256-byte header (b'APP1', size, CRC-32 of the binary, zero padding)."""
import struct
import sys
import zlib


def main() -> int:
    if len(sys.argv) != 3:
        print(__doc__)
        return 2
    with open(sys.argv[1], "rb") as handle:
        body = handle.read()
    header = struct.pack("<4sII", b"APP1", len(body), zlib.crc32(body) & 0xFFFFFFFF).ljust(256, b"\0")
    with open(sys.argv[2], "wb") as handle:
        handle.write(header + body)
    return 0


if __name__ == "__main__":
    sys.exit(main())
