# ASAN-- TestCases

## SPEC CPU2006
SPEC CPU2006 is a set of benchmarks designed to test the CPU performance of a modern server computer system. 

The official website is https://www.spec.org/cpu2006/

Please refer to [spec](https://github.com/junxzm1990/ASAN--/tree/master/testcases/spec) above for detailed reproduce instructions.

## Chromium
Chromium performance under ASan and ASan--. Six popularweb-browser benchmarks are used, including both Time-based ones (Sunspider, Kraken, Lite Brite) and Score-based ones (Octane, Basemark, WebXPRT). And reproduced chromium bugs with PoCs are included.

The official website is https://www.chromium.org/Home

Please refer to [chromium](https://github.com/junxzm1990/ASAN--/tree/master/testcases/chromium) above for detailed reproduce instructions.

## Juliet Test Suite
Juliet Test Suite is a collection of test cases in the C/C++ language. It contains examples for 118 different CWEs. 

The official website is https://samate.nist.gov/SRD/testsuite.php

Please refer to [juliet_test_suite](https://github.com/junxzm1990/ASAN--/tree/master/testcases/juliet_test_suite) above for detailed reproduce instructions.

## Linux Flaw Project
Linux Flaw Project records all the vulnerabilities of linux software that can be reprodeced in local workspace.

The GitHub repo is https://github.com/mudongliang/LinuxFlaw

Please refer to [linux_flaw_project](https://github.com/junxzm1990/ASAN--/tree/master/testcases/linux_flaw_project) above for detailed reproduce instructions.

## Fuzzing
For Fuzzing evaluation, we provided 7 widely used real-world software, which are [objdump](https://www.gnu.org/software/binutils/), [nm](https://www.gnu.org/software/binutils/), [size](https://www.gnu.org/software/binutils/), [cxxfilt](https://www.gnu.org/software/binutils/), [libpng](http://www.libpng.org/pub/png/libpng.html), [file](https://github.com/file/file), [tcpdump](https://www.tcpdump.org/).

Please refer to [fuzzing](https://github.com/junxzm1990/ASAN--/tree/master/testcases/fuzzing) above for detailed fuzzing instructions.

## SanRazor
SanRazor is a sanitizer check reduction tool aiming to incur little overhead while retaining all important sanitizer checks. We also compared ASan-- with SanRazor on SPEC CPU2006. 

The GitHub repo is https://github.com/SanRazor-repo/SanRazor

Please refer to [SanRazor](https://github.com/junxzm1990/ASAN--/tree/master/testcases/SanRazor) above for detailed detailed building instructions.

