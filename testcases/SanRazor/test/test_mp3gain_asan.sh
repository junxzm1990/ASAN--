export PATH=/llvm/build/bin/:$PATH
cd mp3gain-asan
rm -rf mp3gain
rm -rf Profiling
mkdir mp3gain
unzip mp3gain-1_5_2-src.zip -d ./mp3gain
unzip Profiling.zip

cd mp3gain

export SR_STATE_PATH="$(pwd)/Cov"
export SR_WORK_PATH="../coverage.sh"
SanRazor-clang -SR-init
make clean
make CC=SanRazor-clang CXX=SanRazor-clang++ CFLAGS="-Wall -Winline -g -O3 -fsanitize=address"  LIBS="-lm -fsanitize=address" -j 12

cp -r ../Profiling ./
cd Profiling
sudo chmod -R 777 *
./profiling.sh
cd ../

SanRazor-clang -SR-opt -san-level=L$1 -use-asap=1.0
make clean
make CC=SanRazor-clang CXX=SanRazor-clang++ CFLAGS="-Wall -Winline -g -O3 -fsanitize=address"  LIBS="-lm -fsanitize=address" -j 12

cp -r ../cve-test/* ./
bash badtest.sh
