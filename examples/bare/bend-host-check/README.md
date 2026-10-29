# bend-host-check

Bend 2 used where it fits embedded work: on the host, as a reference for firmware logic. `src/ringidx.bend` computes
the ring-buffer index walk; the C firmware (`src/main.c`) does the same in qemu; `test.sh` compares the two.
Bend has no MCU target (its C output needs a hosted POSIX system), so nothing Bend runs on the chip.

Needs `32_31_bend` (`bend-box`, `$BOX_BEND`); without it `needs.sh` exits 3 and check.sh reports SKIP. Written before
that ID existed: the Bend file has not been checked by `bend` yet.

```sh
templates/tools/check.sh --only bend-host-check bare
```
