# ASAN-- TestCases

## Chromium
Chromium performance under ASan and ASan--. Six popularweb-browser benchmarks are used, including both Time-based ones (Sunspider, Kraken, Lite Brite) and Score-based ones (Octane, Basemark, WebXPRT). And reproduced chromium bugs with PoCs are included. 

Please refer to "chromium" for more details.

## Juliet Test Suite
Juliet Test Suite is a collection of test cases in the C/C++ language. It contains examples for 118 different CWEs. 

The official website is https://samate.nist.gov/SRD/testsuite.php

## Linux Flaw Project

Linux Flaw Project records all the vulnerabilities of linux software that can be reprodeced in local workspace.

Details of compiling options are under each CVE folder.

The GitHub repo is https://github.com/mudongliang/LinuxFlaw

## SPEC CPU2006
1. Install [SPEC CPU2006 Benchmark](https://www.spec.org/cpu2006/).

- Please note that we can't share SPEC CPU2006 here becuase it is commercial.

2. Run the following script to run SPEC CPU2006 Benchmark with original ASan under `/cpu2006`:
```
./run_asan.sh asan <test|train|ref> <int|fp>
```
3. Run the following script to run SPEC CPU2006 Benchmark with ASan-- under `/cpu2006`:
```
./run_asan--.sh asan-- <test|train|ref> <int|fp>



