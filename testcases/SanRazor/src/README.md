# Module-level documentation for SanRazor
1. `SCIPass.cpp`: identify all user checks and sanitizer checks.
2. `DynamicCallCounter.cpp`: insert function calls for each user/sanitizer check in order to get its dynmaic patterns.
3. `SRAnalysisPass.cpp`: 1) ananlyze the dynamic patterns and static patterns of user/sanitizer checks; 2) identify redundant sanitizer checks; 3) remove redundant sanitizer checks.
4. `SCClean.cpp`: remove instructions related to removed sanitizer checks.
5. `CostModel.cpp`: analyze the cost of sanitizer checks.
6. `SanRazor-clang.rb`: implement `SanRazor-clang/SanRazor-clang++`.
7. `SanRazor-clang-utils.rb`: useful functions for implementing `SanRazor-clang/SanRazor-clang++`.