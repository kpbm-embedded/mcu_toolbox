# bend-host-check

Bend 2 used where it fits embedded work: on the host, as a reference for firmware logic. `src/ringidx.bend` computes
the ring-buffer index walk; the C firmware (`src/main.c`) does the same in qemu; `test.sh` compares the two.
Bend has no MCU target (its C output needs a hosted POSIX system), so nothing Bend runs on the chip.

Needs the optional pinned Bend 2.0.39 (`mcu setup install bend`, config/pins.tsv, sha256 checked); without it
`needs.sh` exits 3 and check.sh reports SKIP. Bend runs with HOME/TMPDIR in `cache/bend-home` and BEND_NO_TELEMETRY=1.
Bend checks termination left to right, so `walk` takes the shrinking step count first.

```sh
templates/tools/check.sh --only bend-host-check bare
```
