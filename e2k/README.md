# ROOT on e2k (Elbrus)

This branch is ROOT 6.42 (`v6-42-00-patches`) with support for the e2k
(Elbrus) architecture. The interpreter (cling) uses ROOT's builtin LLVM 22
with the e2k support of the MCST
[llvm-project-e-2-k](https://gitflic.ru/project/e2khome/llvm-project-e-2-k)
fork (branch `llvmorg-21-e2k`) forward-ported to it. LLVM IR is translated
to lccrt IR and compiled for e2k by the MCST lccopt optimizer, both ahead of
time and in the cling JIT.

ROOT 6.36 with the earlier LLVM 18 based port is in the `e2k/v6-36` branch.

## Status

Built and tested on Elbrus-8C2 (e2kv5) with lcc 1.31.05 and
lccrt/liblccopt 151384. Everything in the configuration below works,
including the JIT, ACLiC, C++ exceptions through JIT code, clad, PyROOT,
RDataFrame with implicit multi-threading, RNTuple, RooFit, TMVA, graphics,
THttpServer and SQLite (see [Testing](#testing)).

Not available on e2k: `vdt`, `xrootd` and `fitsio` (no packages);
`builtin_*` replacements are used for FTGL, civetweb and GSL.

## Requirements

- `lcc` (MCST C/C++ compiler), `lccrt` and `liblccopt` (151384 or newer);
  `lccrt.h` and `liblccrt.so` under one prefix (normally `/usr`).
- CMake, Ninja, Python 3 with headers, and the usual
  [ROOT build dependencies](https://root.cern/install/dependencies/).
- Additionally used by the configuration below: `gl2ps-devel`,
  `mesa-libGL-devel`, `mesa-libGLU-devel`, `libglvnd-devel`,
  `sqlite-devel`, and `automake` (fresh `config.guess` for builtin GSL).
- Network access during the build (builtin GSL, clad and others are
  downloaded).

## Building

```sh
cmake -S root -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/opt/root \
  -DLLVM_WITH_LCCRT=/usr \
  -DLLVM_PARALLEL_LINK_JOBS=2 \
  -Dvdt=OFF -Dxrootd=OFF -Dfitsio=OFF \
  -Dbuiltin_ftgl=ON -Dbuiltin_civetweb=ON -Dbuiltin_gsl=ON \
  -Dmathmore=ON -Dtmva-cpu=ON -Dsqlite=ON
ninja -C build
```

lcc does not emit DWARF debug information. To keep symbol names for
backtraces in gdb, add `-DCMAKE_STRIP=/bin/true` (ROOT strips libCling in
Release builds).

## Testing

```sh
bash e2k/tests/test-full.sh build        # functional checks, ~25 min
bash e2k/tests/plb2/run.sh build         # plb2 benchmarks, JIT vs ACLiC
```

Both scripts take a build or install directory containing
`bin/thisroot.sh` and exit with a non-zero status on failure.

## e2k specifics

- **JIT optimization level.** cling uses `-O3` on e2k (ROOT's default is
  `-O1`): the code is optimized by lccopt, and its speed depends strongly on
  the level. `-O3` compiles slower; use `EXTRA_CLING_ARGS=-O2` or
  `#pragma cling optimize(2)` where compile time matters more.
- **Pointer checks.** In interactive sessions cling checks pointers before
  dereferencing them. On e2k this is an inline null check: null
  dereferences still raise a cling exception, other invalid pointers end in
  `SIGSEGV`, as in compiled code. `CLING_POINTER_CHECKS=1` restores the full
  check, which is 15-17x slower on e2k.
- **Vectorization** is left to lccopt; the LLVM loop and SLP vectorizers are
  disabled in cling.
- **Incremental rebuilds.** ROOT does not revalidate its precompiled
  modules. After changing cling's runtime headers
  (`interpreter/cling/include/cling/Interpreter/Runtime*.h`), remove
  `build/lib/*.pcm` and `build/lib/modules.idx` and run `ninja` again.

## Performance

plb2 macros on Elbrus-8C2, execution time of the JIT-compiled code
(compilation excluded) compared with the same code built ahead of time:

| | cling JIT `-O3` | lcc `-O3` | MCST clang `-O3` |
|---|---|---|---|
| nqueen | 8.2 s | 10.0 s | 11.9 s |
| matmul | 3.5 s | 2.8 s | 2.5 s |
| sudoku | 13.1 s | 12.0 s | 12.5 s |
| bedcov | 10.2 s | 8.6 s | 10.1 s |

## Patch series

On top of `v6-42-00-patches`, see `git log v6-42-00-patches..e2k/v6-42`:

1. LLVM 22 port of the MCST e2k support (7 commits: TargetParser/ELF/build
   system, Elbrus target, CodeGen/Lccrt translator, IR and optimizer,
   RuntimeDyld, clang target, intrinsic headers).
2. Fixes to code from the MCST fork: `ProgramStack`, lccrt atomic ordering,
   `getGOTEntrySize()`, `R_E2K_32_PC` (broke C++ exceptions in JIT code).
3. ROOT platform support for e2k (CMake, `root-config`, `RConfig.hxx`).
4. Fixes needed to build with lcc or to run with lccrt objects that are not
   e2k specific and could go upstream: MemProf traits, constexpr string
   tables, `__cplusplus` detection, out-of-line destructors, `RPageStorage`,
   compiler ID `LCC` in CMake, CppInterOp resource directory, `STB_GNU_UNIQUE`
   in RuntimeDyld (fixed heap corruption in the JIT).
5. ROOT on e2k: RFloat16, builtin GSL, clad, and the cling settings above.
