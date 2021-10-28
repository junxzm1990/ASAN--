mkdir ASan_Srk && cd ASan_Srk
CC=afl-clang CXX=afl-clang++ LLVM_COMPILER=clang CFLAGS="-fsanitize=address -g -O2" ../configure --enable-shared=no --enable-static=yes
make -j4
