#!/bin/bash
sed -i '7i add_subdirectory(SRPass)' llvm/lib/Transforms/CMakeLists.txt
sed -i "s/static_assert(SmallSize <= .*, \"SmallSize should be small\");/static_assert(SmallSize <= 1024, \"SmallSize should be small\");/g" llvm/include/llvm/ADT/SmallPtrSet.h