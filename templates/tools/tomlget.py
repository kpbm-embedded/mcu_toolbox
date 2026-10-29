#!/usr/bin/env python3
"""tomlget.py <file> <key> : print a top-level TOML key (lists one item per line). A missing key prints nothing.

Used by check.sh to read `targets` from example.toml / template.toml. Standard library only (tomllib).
"""
import argparse
import sys
import tomllib


def main() -> int:
    parser = argparse.ArgumentParser(description="Print one top-level key of a TOML file (lists: one item per line).")
    parser.add_argument("file", help="TOML file, e.g. examples/stm32/blink/example.toml")
    parser.add_argument("key", help="top-level key, e.g. targets")
    args = parser.parse_args()
    with open(args.file, "rb") as handle:
        value = tomllib.load(handle).get(args.key)
    if isinstance(value, list):
        print("\n".join(map(str, value)))
    elif value is not None:
        print(value)
    return 0


if __name__ == "__main__":
    sys.exit(main())
