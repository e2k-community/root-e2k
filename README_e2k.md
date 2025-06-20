## About
This ROOT fork is based on the [official ROOT repository](https://github.com/root-project/root) with patches adding e2k (Elbrus) arch support.

The provided changes contain patches to ```interpreter/llvm-project``` based on the [LLVM porting project to the e2k arch](https://gitflic.ru/project/e2khome/llvm-project-e-2-k).
It also includes patches adding e2k support to Cling and ROOT.

ROOT functionality was tested on Elbrus-8c2 (e2kv5) and Elbrus-2c3 (e2kv6) processors.

## Build requires on e2k
For successful build and operation of ROOT on e2k it is necessary to install extra packages:
- lccrt ≥ 146390
- liblccopt ≥ 146390
- lcc & lcc-libs
- llvm & clang in one of:
  - 13 or 17 from [MCST](https://gitflic.ru/project/e2khome/llvm-project-e-2-k)
  - 18 from [Unipro](https://git.openelbrus.ru/unipro/llvm-e2k)

## Build on e2k

For build on e2k arch it is necessary to use ```-g0``` to produce no debug information and extra cmake flags:
```
-DBUILD_SHARED_LIBS:BOOL=ON
-DCMAKE_BUILD_TYPE=MinSizeRel
-DLLVM_TARGET_ARCH=e2k
-DLLVM_DEFAULT_TARGET_TRIPLE=e2k64-unknown-linux-gnu
-DLLVM_WITH_LCCRT=/usr/lib64/lccrt
-DLLVM_LCCRT_RELEASE=True
-Dclad:BOOL=OFF
-DLLVM_ENABLE_UNWIND_TABLES=OFF
-DBUILD_SHARED_LIBS=ON
```
