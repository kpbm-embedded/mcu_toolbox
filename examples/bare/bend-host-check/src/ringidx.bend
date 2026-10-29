# Host-side reference for the firmware's ring-buffer index math (src/main.c): next(i) = (i + 1) % size.
# Bend 2 runs on the host only (no MCU target); test.sh compares its output with the firmware's.
# Written before 32_31_bend existed: syntax follows the Bend 2 README and is not yet checked by `bend`.
def next(i: U32, size: U32) -> U32:
  (i + 1) % size

def walk(i: U32, size: U32, steps: U32) -> U32:
  if steps == 0:
    i
  else:
    walk(next(i, size), size, steps - 1)

def main() -> U32:
  walk(0, 8, 1000)
