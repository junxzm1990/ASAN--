## SPEC CPU2006
1. Install [SPEC CPU2006 Benchmark](https://www.spec.org/cpu2006/).
- Please note that we can't share SPEC CPU2006 here becuase it is commercial.
2. After you obtained SPEC CPU2006 Benchmark:
```
$ cp run_asan--.sh /cpu2006
$ cd cpu2006
```
3. Run the following script to run SPEC CPU2006 Benchmark with ASan-- under `/cpu2006`:
```
~/cpu2006 $ ./run_asan--.sh asan-- <test|train|ref> <int|fp>
```

