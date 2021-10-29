#!/bin/bash
cd ../../../SPEC_CPU2006v1.0
source shrc
cd ../SanRazor/data/spec

echo "Running spec with SanRazor+ASan+L0 >>>"
./run_spec_SR.sh asan L0 $1 &> asan_L0.txt

echo "Running spec with SanRazor+ASan+L1 >>>"
./run_spec_SR.sh asan L1 $1 &> asan_L1.txt

echo "Running spec with SanRazor+ASan+L2 >>>"
./run_spec_SR.sh asan L2 $1 &> asan_L2.txt

echo "Running spec with SanRazor+UBSan+L0 >>>"
./run_spec_SR.sh ubsan L0 $1 &> ubsan_L0.txt

echo "Running spec with SanRazor+UBSan+L1 >>>"
./run_spec_SR.sh ubsan L1 $1 &> ubsan_L1.txt

echo "Running spec with SanRazor+UBSan+L2 >>>"
./run_spec_SR.sh ubsan L2 $1 &> ubsan_L2.txt

echo "Running spec with no sanitizer >>>"
./run_spec.sh default $1 &> default.txt

echo "Running spec with ASan >>>"
./run_spec.sh asan $1 &> asan.txt

echo "Running spec with ubSan >>>"
./run_spec.sh ubsan $1 &> ubsan.txt

echo "Generate M1/M2 results for spec >>>"
python extract.py --spec_path $2 --setup SR_asan_L0
python extract.py --spec_path $2 --setup SR_asan_L1
python extract.py --spec_path $2 --setup SR_asan_L2
python extract.py --spec_path $2 --setup SR_ubsan_L0
python extract.py --spec_path $2 --setup SR_ubsan_L1
python extract.py --spec_path $2 --setup SR_ubsan_L2