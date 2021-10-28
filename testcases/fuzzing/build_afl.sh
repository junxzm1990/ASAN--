cd afl-2.52b
CC=../../../llvm-4.0.0-project/ASan_Build/bin/clang CXX=../../../llvm-4.0.0-project/ASan_Build/bin/clang++ make -j4
sudo make install

sudo update-alternatives --install /usr/local/bin/clang clang $(readlink -f ../llvm-4.0.0-project/ASan--Build/bin)/clang 250
sudo update-alternatives --install /usr/local/bin/clang++ clang++ $(readlink -f ../llvm-4.0.0-project/ASan--Build/bin)/clang++ 250