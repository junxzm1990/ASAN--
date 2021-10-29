# SanRazor Building Instructions
## Overview
SanRazor is a sanitizer check reduction tool aiming to incur little overhead while retaining all important sanitizer checks. 

## Getting Started Instructions
### Quickly install & test using docker
```
docker build -f Dockerfile -t sanrazor:latest --shm-size=8g . 
docker run -it sanrazor:latest
bash test_autotrace.sh
```

Note that this docker image is publicly available [here](https://hub.docker.com/r/sanrazor/sanrazor-snapshot), and it contains prebuilt LLVM9 and SanRazor. To build it from scratch, you can use `Dockerfile_sanrazor`.

## Detailed Instructions
### 1. Prerequisite
```
wget xz-utils cmake make g++ python3 python3-distutils
```
### 2. Install
2.1. Download and install [LLVM](https://llvm.org/docs/GettingStarted.html) and [Clang](https://clang.llvm.org/get_started.html).
Run the following command in Ubuntu 18.04/20.04 to complete this step:
```
./download_llvm9.sh
```

Note that LLVM9 can not be built on Ubuntu20.04 due to an incompatibility with glibc 2.31 (see [LLVM PR D70662](https://reviews.llvm.org/D70662)). To quickly fix it, you can run the following lines after you download the LLVM9 source code:
```
sed -i 's/unsigned short mode;/unsigned int mode;/g' compiler-rt/lib/sanitizer_common/sanitizer_platform_limits_posix.h
sed -i '/unsigned short __pad1;/d' compiler-rt/lib/sanitizer_common/sanitizer_platform_limits_posix.h
``` 
or 
```
pushd llvm/projects/compiler-rt/lib/sanitizer_common
sed -e '1131 s|^|//|' \
    -i sanitizer_platform_limits_posix.cc
popd
```

2.2. Move the source code of SanRazor into your llvm project:
```
cp -r src/SRPass llvm/lib/Transforms/
```

2.3. Run the following command to change `CMakeLists.txt` and `SmallPtrSet.h` (also see `src/patch.sh`):
```
sed -i '7i add_subdirectory(SRPass)' llvm/lib/Transforms/CMakeLists.txt
sed -i "s/static_assert(SmallSize <= .*, \"SmallSize should be small\");/static_assert(SmallSize <= 1024, \"SmallSize should be small\");/g" llvm/include/llvm/ADT/SmallPtrSet.h
```

2.4. Compile your llvm project again:
```
./build_and_install_llvm9.sh
```

2.5. Install [ruby](https://www.ruby-lang.org/en/documentation/installation/) and make sure that the following libraries are installed in your system:
```
gem install fileutils
gem install parallel
gem install pathname
gem install shellwords
```

### 3. Usage of SanRazor
3.1. Initialization by the following code:
```
export SR_STATE_PATH="$(pwd)/Cov"
export SR_WORK_PATH="<path-to-your-coverage.sh>/coverage.sh"
SanRazor-clang -SR-init
```

3.2. Set your compiler for C/C++ program as `SanRazor-clang`/`SanRazor-clang++` (`CC=SanRazor-clang`/`CXX=SanRazor-clang++`), and run the following command:
```
make CC=SanRazor-clang CXX=SanRazor-clang++ CFLAGS="..." CXXFLAGS="..." LDFLAGS="..." -j $(nproc)
```

3.3. Run your program with workload. The profiling result will be written into folder `$(pwd)/Cov`.

3.4. Run the following command to perform sanitizer check reduction (Note that we provide the option of using ASAP first with `asap_budget` and running SanRazor later. If you do not want to use ASAP, set `-use-asap=1.0`):
```
make clean
SanRazor-clang -SR-opt -san-level=<L0/L1/L2> -use-asap=<asap_budget>
make CC=SanRazor-clang CXX=SanRazor-clang++ CFLAGS="..." CXXFLAGS="..." LDFLAGS="..." -j $(nproc)
```

3.5. Test your program after check reduction.

### 4. Reproducing SPEC results
4.1. Install [SPEC CPU2006 Benchmark](https://www.spec.org/cpu2006/).

4.2. Run the following code under `SPEC_CPU2006v1.0/` to activate the spec environment:
```
source shrc
```

4.3. Run the following script to run SPEC CPU2006 Benchmark with SanRazor+ASan/UBSan under `data/spec/`:
```
./run_spec_SR.sh <asan/ubsan> <L0/L1/L2> <test/ref>
```

4.4. Run the following script to run SPEC CPU2006 Benchmark without SanRazor under `data/spec/`:
```
./run_spec.sh <asan/ubsan/default> <test/ref>
```

4.5. See the evaluation reports under `SPEC_CPU2006v1.0/result`.
