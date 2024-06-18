# ASAN--

## Note: The implementation based on LLVM-12 is preliminary and not mature. If you want to run ASan-- as a baseline, please use the LLVM-4-based version.

## Overview
AddressSanitizer (ASan) is a powerful memory error detector. It can detect various errors ranging from spatial issues like out-of-bound accesses to temporal issues like use-after-free. However, ASan has the major drawback of high runtime overhead. In order to reduce the overhead, we propose ASan--, a tool assembling a group of optimizations to reduce (or “debloat”) sanitizer checks and improve ASan’s efficiency without harming the capability, scalability, or usability.

You can find the source code to implement each of our optimizations below:

- Removing Unsatisfiable Checks

	- Global Optimization [[src]](https://github.com/junxzm1990/ASAN--/blob/64b72d964a1f1542f7341774980a43ddd6fbf189/llvm-4.0.0-project/llvm/lib/Transforms/Instrumentation/AddressSanitizer.cpp#L1385)
	- Stack Optimization [[src]](https://github.com/junxzm1990/ASAN--/blob/64b72d964a1f1542f7341774980a43ddd6fbf189/llvm-4.0.0-project/llvm/lib/Transforms/Instrumentation/AddressSanitizer.cpp#L1404)

- Removing Recurring Checks [[src]](https://github.com/junxzm1990/ASAN--/blob/64b72d964a1f1542f7341774980a43ddd6fbf189/llvm-4.0.0-project/llvm/lib/Transforms/Instrumentation/AddressSanitizer.cpp#L3212)

- Optimizing Neighbor Checks [[src]](https://github.com/junxzm1990/ASAN--/blob/64b72d964a1f1542f7341774980a43ddd6fbf189/llvm-4.0.0-project/llvm/lib/Transforms/Instrumentation/AddressSanitizer.cpp#L3217)

- Optimizing Checks in Loops [[src]](https://github.com/junxzm1990/ASAN--/blob/64b72d964a1f1542f7341774980a43ddd6fbf189/llvm-4.0.0-project/llvm/lib/Transforms/Instrumentation/AddressSanitizer.cpp#L3220)

## Building Environment
ASan-- is supported by different Ubuntu versions. For reproductive experiments, we recommend you to build ASan-- on Ubuntu 18.04 LTS 64bit (a virtual machine is fine). To support the benchmarks testing Chromium, we suggest you to install the desktop version of Ubuntu.

Before you can compile ASAN--, you will need to install the following dependencies:
```
$ sudo apt-get install cmake
$ sudo apt-get install git
$ sudo apt-get install wget
$ sudo apt-get install tar
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
$ cd vanilla_llvm
$ mkdir ASan_Build && cd ASan_Build
$ cmake -DLLVM_ENABLE_PROJECTS="clang;compiler-rt" -G "Unix Makefiles" ../llvm
$ make -j
```
## Test Cases
For evaluation part, we used [SPEC CPU2006 Benchmark](https://www.spec.org/cpu2006/) and [Chromium Project](https://www.chromium.org/Home) to evaluate the runtime performance, then utilized [Juliet Test Suite](https://samate.nist.gov/SRD/testsuite.php) and [Linux Flaw Project](https://github.com/mudongliang/LinuxFlaw) to evaluate the bug detection capability. 

For more details, please refer to Section 5 "Implementation and Evaluation" in our paper.

### Reproduce Experiment Instuctions
- Please see [SPEC CPU2006](https://github.com/junxzm1990/ASAN--/tree/master/testcases/spec)
- Please see [Chromium Project](https://github.com/junxzm1990/ASAN--/tree/master/testcases/chromium)
- Please see [Juliet Test Suite](https://github.com/junxzm1990/ASAN--/tree/master/testcases/juliet_test_suite)
- Please see [Linux Flaw Project](https://github.com/junxzm1990/ASAN--/tree/master/testcases/linux_flaw_project)

### Fuzzing
For fuzzing part, we implemented two versions. ASan-- integrating FuZZan version and only ASan-- version. Please run patches below separately before starting each fuzzing process:
```
$ patch -p1 < patch_ASan--FuZZan
$ cd llvm-4.0.0-project
$ mkdir ASan--Build && cd ASan--Build
$ cmake -DLLVM_ENABLE_PROJECTS="clang;compiler-rt" -G "Unix Makefiles" ../llvm
$ make -j
```
Or

```
$ patch -p1 < patch_ASan--
$ cd llvm-4.0.0-project
$ mkdir ASan--Build && cd ASan--Build
$ cmake -DLLVM_ENABLE_PROJECTS="clang;compiler-rt" -G "Unix Makefiles" ../llvm
$ make -j
```
Please see [Fuzzing](https://github.com/junxzm1990/ASAN--/tree/master/testcases/fuzzing) for detailed fuzzing instructions.

### SanRazor
We also include the comparison between ASan-- and SanRazor on SPEC CPU2006 in our evaluation. 

Please see [SanRazor](https://github.com/junxzm1990/ASAN--/tree/master/testcases/SanRazor) for detailed building instructions.

## If you do not want to build ASAN-- from scratch, you can use the dockers we prepared:
Ubuntu 18.04 Docker:
```console
$ docker build -f Dockerfile_1804 -t asanopt:latest --shm-size=100g .
$ docker run -it asanopt:latest
REPOSITORY          TAG                 IMAGE ID            CREATED             SIZE
asanopt             latest              8d74111c5249        About an hour ago   55.4GB
```
Ubuntu 16.04 Docker(For building Chromium):
```console
$ docker build -f Dockerfile_1604 -t optasan-1604:latest --shm-size=100g .
$ docker run -it optasan-1604:latest
REPOSITORY          TAG                 IMAGE ID            CREATED             SIZE
optasan-1604        latest              de02e86743ea        About an hour ago   55.4GB
```
Location of ASan--:
```
/home/llvm-4.0.0-project/ASan--Build/bin/<clang|clang++>
```
Location of Self-Built LLVM/Clang:
```
/home/original_llvm/ASan--Build/bin/<clang|clang++>
```
Location of Pre-Built LLVM/Clang:
```
/usr/bin/<clang-4.0|clang++-4.0>
```

We also provided the testcases inside Docker.

- Location of SPEC CPU2006
```
/home/testcases/spec
```
To reproduce, please follow the instructions [here](https://github.com/junxzm1990/ASAN--/tree/master/testcases/spec)

- Location of Chromium Project
```
/home/testcases/chromium
```
To reproduce, please follow the instructions [here](https://github.com/junxzm1990/ASAN--/tree/master/testcases/chromium)

- Location of Juliet Test Suite:
```
/home/testcases/juliet_test_suite
```
To reproduce, please follow the instructions [here](https://github.com/junxzm1990/ASAN--/tree/master/testcases/juliet_test_suite)

- Location of Linux Flaw Project:
```
/home/testcases/linux_flaw_project
```
To reproduce, please follow the instructions [here](https://github.com/junxzm1990/ASAN--/tree/master/testcases/linux_flaw_project)

Please note the [docker image](https://hub.docker.com/repository/docker/yzhang71/optasan-1604) is publicly available, and it contains prebuilt ASAN-- and testcases. To build it from scratch, you can use Dockerfile_ASAN-- with commands below:
```
$ docker build -f Dockerfile_ASAN-- -t asanopt:latest --shm-size=100g .
$ docker run -it asanopt:latest
```
