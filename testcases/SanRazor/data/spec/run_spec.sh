#!/bin/bash

set -e

ALL_BENCHMARKS="
   401.bzip2
   429.mcf
   445.gobmk
   456.hmmer
   458.sjeng
   462.libquantum
   433.milc
   470.lbm
   482.sphinx3
   444.namd
   453.povray
"

#no ubsan h264ref
if ! which runspec > /dev/null; then
    echo "Please run \"source shrc\" in the spec folder prior to calling this script." >&2
    exit 1
fi
export PATH=~/workspace/llvm/build/bin/:$PATH
export SR_WORK_PATH="$(pwd)/coverage.sh"
export ASAN_OPTIONS=alloc_dealloc_mismatch=0:detect_leaks=0:halt_on_error=0
export UBSAN_OPTIONS=halt_on_error=0

runspec --config="$(pwd)/SR_off.cfg" --rebuild --extension="$1"  --noreportable --size=$2 ${ALL_BENCHMARKS}
