## SPEC CPU2006
1. Install [SPEC CPU2006 Benchmark](https://www.spec.org/cpu2006/).
- Please note that we can't share SPEC CPU2006 here becuase it is commercial.
2. After you obtained SPEC CPU2006 Benchmark (**assuming you place `cpu2006` under the folder of `spec`**):
```
$ cp run_asan--.sh ./cpu2006
$ cd cpu2006
```
3. Run the following script to run SPEC CPU2006 Benchmark with ASan-- (inside the folder of`cpu2006`):
```
$ CC=../../../llvm-4.0.0-project/ASan--Build/bin/clang CXX=../../../llvm-4.0.0-project/ASan--Build/bin/clang++ ./run_asan--.sh asan-- <test|train|ref> <int|fp>
```
4. To run multiple rounds, please set number "X" to "-n". For example, run 3 rounds (inside the folder of`cpu2006`):
```
$ CC=../../../llvm-4.0.0-project/ASan--Build/bin/clang CXX=../../../llvm-4.0.0-project/ASan--Build/bin/clang++ ./run_asan--.sh asan-- <test|train|ref> <int|fp> -n 3
```
