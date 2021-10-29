#!/bin/bash

wget https://github.com/llvm/llvm-project/releases/download/llvmorg-9.0.1/llvm-9.0.1.src.tar.xz
tar -xf llvm-9.0.1.src.tar.xz
rm -rf llvm-9.0.1.src.tar.xz
mv llvm-9.0.1.src llvm
cd llvm/tools

wget https://github.com/llvm/llvm-project/releases/download/llvmorg-9.0.1/clang-9.0.1.src.tar.xz
tar -xf clang-9.0.1.src.tar.xz
mv clang-9.0.1.src clang
cd clang/tools

wget https://github.com/llvm/llvm-project/releases/download/llvmorg-9.0.1/clang-tools-extra-9.0.1.src.tar.xz
tar -xf clang-tools-extra-9.0.1.src.tar.xz
rm -rf clang-tools-extra-9.0.1.src.tar.xz
mv clang-tools-extra-9.0.1.src extra

cd ../../..
cd projects

wget https://github.com/llvm/llvm-project/releases/download/llvmorg-9.0.1/compiler-rt-9.0.1.src.tar.xz
wget https://github.com/llvm/llvm-project/releases/download/llvmorg-9.0.1/libcxx-9.0.1.src.tar.xz
wget https://github.com/llvm/llvm-project/releases/download/llvmorg-9.0.1/libcxxabi-9.0.1.src.tar.xz

tar -xf compiler-rt-9.0.1.src.tar.xz
tar -xf libcxx-9.0.1.src.tar.xz
tar -xf libcxxabi-9.0.1.src.tar.xz
rm -rf *.src.tar.xz
mv compiler-rt-9.0.1.src compiler-rt
mv libcxx-9.0.1.src libcxx
mv libcxxabi-9.0.1.src libcxxabi