#!/bin/bash

mkdir original_llvm && cd original_llvm

wget https://releases.llvm.org/4.0.0/llvm-4.0.0.src.tar.xz
wget https://releases.llvm.org/4.0.0/cfe-4.0.0.src.tar.xz
wget https://releases.llvm.org/4.0.0/compiler-rt-4.0.0.src.tar.xz

sudo apt-get install tar

tar -xvf llvm-4.0.0.src.tar.xz
tar -xvf cfe-4.0.0.src.tar.xz
tar -xvf compiler-rt-4.0.0.src.tar.xz

rm llvm-4.0.0.src.tar.xz
rm cfe-4.0.0.src.tar.xz
rm compiler-rt-4.0.0.src.tar.xz

mkdir ASan_Build && cd ASan_Build

cmake -DLLVM_ENABLE_PROJECTS="cfe-4.0.0.src;compiler-rt-4.0.0.src" -G "Unix Makefiles" ../llvm-4.0.0.src
make -j



