# ASAN--

## Overview
AddressSanitizer (ASan) is a powerful memory error detector. It can detect various errors ranging from spatial issues like out-of-bound accesses to temporal issues like use-after-free. However, ASan has the major drawback of high runtime overhead. In order to reduce the overhead, we propose ASan--, a tool assembling a group of optimizations to reduce (or “debloat”) sanitizer checks and improve ASan’s efficiency without harming the capability, scalability, or usability.

## Environment
Ubuntu 18.04 LTS 64bit

## Build Vanilla LLVM
```
$ ./vanilla_llvm_autosetup.sh
```

## Build ASan--enabled LLVM
```
$ git clone https://github.com/junxzm1990/ASAN--.git
$ mkdir ASan--Build && cd ASan--Build
$ cmake -DLLVM_ENABLE_PROJECTS="clang;compiler-rt" -G "Unix Makefiles" ../llvm
$ make -j
```

## Run ASan-- on SPEC2006
1. Install [SPEC CPU2006 Benchmark](https://www.spec.org/cpu2006/).
2. Run the following script to run SPEC CPU2006 Benchmark with original ASan under `/cpu2006`:
```
./run_asan.sh original_asan <test|train|ref> int
```
3. Run the following script to run SPEC CPU2006 Benchmark with ASan-- under `/cpu2006`:
```
./run_asan--.sh asan-- <test|train|ref> int
```


