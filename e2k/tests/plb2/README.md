# plb2 macros

ROOT macro versions of the [plb2](https://github.com/attractivechaos/plb2)
benchmarks, taken from [e2k-community/plb2](https://github.com/e2k-community/plb2)
(`src/root`; `sudoku.C` is the C version with `main` renamed to `sudoku()`).
plb2 is released under CC0 1.0; `sudoku.C` carries its own MIT license header.

`run.sh ROOT_DIR` runs every macro with the cling JIT and with ACLiC and checks
that both produce the same output.
