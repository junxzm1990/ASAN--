# ASAN--

## Overview
AddressSanitizer (ASan) is a powerful memory error detector. It can detect various errors ranging from spatial issues like out-of-bound accesses to temporal issues like use-after-free. However, ASan has the major drawback of high runtime overhead. In order to reduce the overhead, we propose ASan--, a tool assembling a group of optimizations to reduce (or “debloat”) sanitizer checks and improve ASan’s efficiency without harming the capability, scalability, or usability.

You can find the source code to implement each of our optimizations below:

- Removing Unsatisfiable Checks

	- [Global Optimization](https://github.com/junxzm1990/ASAN--/blob/64b72d964a1f1542f7341774980a43ddd6fbf189/llvm-4.0.0-project/llvm/lib/Transforms/Instrumentation/AddressSanitizer.cpp#L1385) Source code of optimization can be found here.
	- [Stack Optimization](https://github.com/junxzm1990/ASAN--/blob/64b72d964a1f1542f7341774980a43ddd6fbf189/llvm-4.0.0-project/llvm/lib/Transforms/Instrumentation/AddressSanitizer.cpp#L1404) Source code of optimization can be found here.


- [Removing Recurring Checks](https://github.com/junxzm1990/ASAN--/blob/64b72d964a1f1542f7341774980a43ddd6fbf189/llvm-4.0.0-project/llvm/lib/Transforms/Instrumentation/AddressSanitizer.cpp#L3212) Source code of optimization can be found here.

- [Optimizing Neighbor Checks](https://github.com/junxzm1990/ASAN--/blob/64b72d964a1f1542f7341774980a43ddd6fbf189/llvm-4.0.0-project/llvm/lib/Transforms/Instrumentation/AddressSanitizer.cpp#L3217) Source code of optimization can be found here.

- [Optimizing Checks in Loops](https://github.com/junxzm1990/ASAN--/blob/64b72d964a1f1542f7341774980a43ddd6fbf189/llvm-4.0.0-project/llvm/lib/Transforms/Instrumentation/AddressSanitizer.cpp#L3220) Source code of optimization can be found here.

## Building Environment
ASan-- is supported by different Ubuntu versions. For reproductive experiments, we recommend you to build ASan-- on Ubuntu 18.04 LTS 64bit (a virtual machine is fine). To support the benchmarks testing Chromium, we suggest you to install the desktop version of Ubuntu.

Before you can compile ASAN--, you will need to install the following dependencies:
```
$ sudo apt-get install cmake
$ sudo apt-get install git
$ sudo apt-get install wget
```

## Build ASan-- from source code
```
$ git clone https://github.com/junxzm1990/ASAN--.git && cd ASAN--
$ cd llvm-4.0.0-project
$ mkdir ASan--Build && cd ASan--Build
$ cmake -DLLVM_ENABLE_PROJECTS="clang;compiler-rt" -G "Unix Makefiles" ../llvm
$ make -j
```

## Build Vanilla LLVM
In case you want to run the original LLVM-4.0.0 for comparison, please run:
```
$ sudo apt-get install tar
$ ./vanilla_llvm_autosetup.sh
```
## Test Cases
For evaluation part, we used [SPEC CPU2006 Benchmark](https://www.spec.org/cpu2006/) and [Chromium Project](https://www.chromium.org/Home) to evaluate the runtime performance, then utilized [Juliet Test Suite](https://samate.nist.gov/SRD/testsuite.php) and [Linux Flaw Project](https://github.com/mudongliang/LinuxFlaw) to evaluate the bug detection capability. 

For more details, please refer to Section 5 "Implementation and Evaluation" in our paper. 

### Reproduce Experiment Instuctions
- Please see [SPEC CPU2006](https://github.com/junxzm1990/ASAN--/tree/master/testcases/spec)
- Please see [Chromium Project](https://github.com/junxzm1990/ASAN--/tree/master/testcases/chromium)
- Please see [Juliet Test Suite](https://github.com/junxzm1990/ASAN--/tree/master/testcases/juliet_test_suite)
- Please see [Linux Flaw Project](https://github.com/junxzm1990/ASAN--/tree/master/testcases/linux_flaw_project)



## If you do not want to build ASAN-- from scratch, you can use the docker we prepared:
```
$ docker build -f Dockerfile_ASAN-- -t asanopt:latest --shm-size=100g .
$ docker run -it asanopt:latest
```
