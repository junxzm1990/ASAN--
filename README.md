# ASAN--

## Overview
AddressSanitizer (ASan) is a powerful memory error detector. It can detect various errors ranging from spatial issues like out-of-bound accesses to temporal issues like use-after-free. However, ASan has the major drawback of high runtime overhead. In order to reduce the overhead, we propose ASan--, a tool assembling a group of optimizations to reduce (or “debloat”) sanitizer checks and improve ASan’s efficiency without harming the capability, scalability, or usability.

## Environment
ASan-- is supported by different Ubuntu versions. For reproductive experiments, we recommend you to build ASan-- on Ubuntu 18.04 LTS 64bit.

## ASan-- Debloating Techniques
- [Removing Unsatisfiable Checks](https://github.com/junxzm1990/ASAN--/blob/e96d4aa82072546e8f2016cf83beba88af4995ea/llvm-4.0.0-project/llvm/lib/Transforms/Instrumentation/AddressSanitizer.cpp#L1385)
- [Removing Recurring Checks](https://github.com/junxzm1990/ASAN--/blob/e96d4aa82072546e8f2016cf83beba88af4995ea/llvm-4.0.0-project/llvm/lib/Transforms/Instrumentation/AddressSanitizer.cpp#L3212)
- [Optimizing Neighbor Checks](https://github.com/junxzm1990/ASAN--/blob/e96d4aa82072546e8f2016cf83beba88af4995ea/llvm-4.0.0-project/llvm/lib/Transforms/Instrumentation/AddressSanitizer.cpp#L3217)
- [Optimizing Checks in Loops](https://github.com/junxzm1990/ASAN--/blob/e96d4aa82072546e8f2016cf83beba88af4995ea/llvm-4.0.0-project/llvm/lib/Transforms/Instrumentation/AddressSanitizer.cpp#L3219)

## Build ASan-- LLVM
```
$ git clone https://github.com/junxzm1990/ASAN--.git
$ cd llvm-4.0.0-project
$ mkdir ASan--Build && cd ASan--Build
$ cmake -DLLVM_ENABLE_PROJECTS="clang;compiler-rt" -G "Unix Makefiles" ../llvm
$ make -j
```

## Build Vanilla LLVM
In case you want to use the original LLVM-4.0.0, please run:
```
$ ./vanilla_llvm_autosetup.sh
```

## Test Cases
For evaluation part, we used [SPEC CPU2006 Benchmark](https://www.spec.org/cpu2006/) and [Chromium](https://www.chromium.org/Home) to evaluate the runtime performance, and we also utilized [Juliet Test Suite](https://samate.nist.gov/SRD/testsuite.php) and [Linux Flaw Project](https://github.com/mudongliang/LinuxFlaw) to evaluate the bug detection capability. 

For more details, please refer to Section 5 "Implementation and Evaluation" in our paper. 

# Reproduce Experiment Instuctions

## Run ASan-- on SPEC2006
1. Install [SPEC CPU2006 Benchmark](https://www.spec.org/cpu2006/).

- Please note that we can't share SPEC CPU2006 becuase it is commercial.

2. Run the following script to run SPEC CPU2006 Benchmark with original ASan under `/cpu2006`:
```
./run_asan.sh asan <test|train|ref> <int|fp>
```
3. Run the following script to run SPEC CPU2006 Benchmark with ASan-- under `/cpu2006`:
```
./run_asan--.sh asan-- <test|train|ref> <int|fp>
```

## Run ASan-- on Chromium
1. Clone the depot_tools repository:
```
$ git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
```
2. Add depot_tools to the end of your PATH :
```
$ export PATH="$PATH:/path/to/depot_tools"
```
3. Create a chromium directory for the checkout and change to it
```
$ mkdir ~/chromium && cd ~/chromium
```
4. Run the fetch tool from depot_tools to check out the code and its dependencies.
```
$ fetch --nohooks chromium
```
5. Check out version 58 of target chromium.
```
$ git checkout tags/58.0.3003.0 -b 58
```
6. Check out a version of depot_tools from around the same time as the target revision.
```
# Get date of current revision:
~/chrome/src $ COMMIT_DATE=$(git log -n 1 --pretty=format:%ci)

# Check out depot_tools revision from the same time:
~/depot_tools $ git checkout $(git rev-list -n 1 --before="$COMMIT_DATE" main)
~/depot_tools $ export DEPOT_TOOLS_UPDATE=0
```
7. Checkout all the submodules at their branch DEPS revisions.
```
$ gclient sync -D --force --reset --with_branch_heads
```
8. To create a build directory, run:
```
gn args out/<ASan|ASan-->
```
9. Set build arguments.
```
is_clang = true
clang_base_path = "/ASAN--/llvm-4.0.0-project/ASan--Build"
is_asan = true
is_debug = ture
symbol_level = 1
is_component_build = true
pdf_use_skia=true
```
10. Build Chromium (the “chrome” target) with Ninja using the command:
```
ninja -C out/<ASan|ASan--> chrome
```

/ASAN--/llvm-4.0.0-project/ASan--Build/bin
