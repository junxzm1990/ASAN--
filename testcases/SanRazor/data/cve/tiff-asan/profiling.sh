#!/bin/bash
#set -e

# Please unzip tif.zip and put it under ./tiff-4.0.7/test folder
# This script should run in ./tiff-4.0.7
# You can ignore any errors generated during profiling.

path="./test/tif/"
cd ./test/tif
testfiles=$(ls *.tif)
cd ../../
for element in $testfiles
do
export ASAN_OPTIONS=allocator_may_return_null=1:detect_leaks=0:halt_on_error=1
echo $path$element
./tools/tiffcrop -i  $path$element /tmp/foo
./tools/tiffsplit $path$element
./tools/tiffcp -i  $path$element /tmp/foo
# ./tools/tiffcp -c g3  $path$element 
# ./tools/tiffcp -c g4  $path$element 
done
