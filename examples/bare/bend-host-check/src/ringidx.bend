import Base
# Host-side reference for the firmware's ring-buffer index math (src/main.c): next(i) = (i + 1) % size.
# Bend 2 runs on the host only (no MCU target); test.sh compares its output with the firmware's.
def next(+i: U32, +size: U32) -> U32:
  ((i + 1) % size : U32)

# steps comes first: Bend checks termination left to right (each argument unchanged until one shrinks)
def walk(steps: Nat, +i: U32, +size: U32) -> U32:
  match steps:
    case 0n:
      i
    case 1n+s:
      walk(s, next(i, size), size)

def main() -> U32:
  walk(1000n, 0, 8)
