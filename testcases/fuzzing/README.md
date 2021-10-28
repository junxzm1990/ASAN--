## Fuzzing Test Cases
1. Build afl-2.52b and set ASan--:
```
$ bash build_afl.sh
```
2. Build libshrink to shrink address space:
```
cd libshrink
./build.sh
```
3. Build fuzzing target softwares:
For each program, we have prepared an "auto_build.sh" script under the source code file. Here is an example to build binutils-2.32.
```
$ cd binutils-2.32
$ bash auto_build.sh
```
4. Reduce the address space for each fuzzing target:
```
$ cd libshrink
$ bash auto_wrap.sh
```
5. Start fuzzing, Weee!
For each software, we have prepared the fuzzing script in "fuzzing_script". Here is an exaple to fuzz "nm":
```
./afl-2.52b/afl-fuzz -S nm_afl -i ./afl-2.52b/testcases/others/elf/ -o ./eval/nm -m none -- libshrink/prelink-nm/nm-new @@
```
The results will be under "eval" folder.
