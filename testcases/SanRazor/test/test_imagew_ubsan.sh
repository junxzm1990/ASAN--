export PATH=/llvm/build/bin/:$PATH
cd imagew-ubsan
rm -rf imageworsener-1.3.1
rm -rf Profiling
tar -xzf 1.3.1.tar.gz
unzip Profiling.zip

cd imageworsener-1.3.1
cp ./scripts/autogen.sh autogen.sh
sudo apt-get install autoconf autotools-dev libtool
./autogen.sh
./configure
export SR_STATE_PATH="$(pwd)/Cov"
export SR_WORK_PATH="../coverage.sh"
SanRazor-clang -SR-init
make clean
make CC=SanRazor-clang CXX=SanRazor-clang++ CFLAGS="-Wall -Winline -g -O3 -fsanitize=undefined"  LDFLAGS="-fsanitize=undefined" -j 12

sudo chmod -R 777 *
cp -r ../Profiling/tests/runtest ./tests
cd tests
./runtest
./runtest
./runtest
cd ../

SanRazor-clang -SR-opt -san-level=L$1 -use-asap=1.0
make clean
make CC=SanRazor-clang CXX=SanRazor-clang++ CFLAGS="-Wall -Winline -g -O3 -fsanitize=undefined"  LDFLAGS="-fsanitize=undefined" -j 12

cp -r ../cve-test/* ./
bash badtest.sh
