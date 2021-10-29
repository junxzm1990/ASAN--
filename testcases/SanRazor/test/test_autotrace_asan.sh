export PATH=/llvm/build/bin/:$PATH
cd auto-asan
rm -rf autotrace-0.31.1
rm -rf Profiling
tar -xzf autotrace-0.31.1.tar.gz
unzip Profiling.zip

cd autotrace-0.31.1
./configure
export SR_STATE_PATH="$(pwd)/Cov"
export SR_WORK_PATH="../coverage.sh"
SanRazor-clang -SR-init
make clean
make CC=SanRazor-clang CXX=SanRazor-clang++ CFLAGS="-Wall -Winline -g -O3 -fsanitize=address"  LDFLAGS="-fsanitize=address" -j 12

cp -r ../Profiling ./
cd Profiling
./profiling.sh
cd ../
SanRazor-clang -SR-opt -san-level=L$1 -use-asap=0.0
make clean
make CC=SanRazor-clang CXX=SanRazor-clang++ CFLAGS="-Wall -Winline -g -O3 -fsanitize=address"  LDFLAGS="-fsanitize=address" -j 12

cp -r ../cve-test/bad ./
cp ../cve-test/badtest.sh ./
bash badtest.sh
