# ASAN--

## Overview
Address Sanitizer (ASan) is a powerful memory error detector. It can detect various errors ranging from spatial issueslike out-of-bound accesses to temporal issues like use-after-free. However, ASan has the major drawback of high runtime overhead. In order to reduce to overhead, we propose ASan--, a tool assembling a group of optimizations to reduce (or “debloat”) sanitizer checks and improve ASan’s efficiency without harming the capability, scalability, or usability of ASan.

## Environment
Ubuntu 18.04 LTS 64bit

## Build ASan--enabled LLVM
```
$ git clone https://github.com/junxzm1990/ASAN--.git
$ mkdir ASan--Build && cd ASan--Build
$ cmake -DLLVM_ENABLE_PROJECTS="clang;compiler-rt" -G "Unix Makefiles" ../llvm
$ make -j
```

