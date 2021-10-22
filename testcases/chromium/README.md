# Chromium Test Cases
## Build Chromium with ASan--
1. Clone the depot_tools repository:
```
$ git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
```
2. Add depot_tools to the end of your PATH :
```
$ export PATH="$PATH:/path/to/depot_tools"
```
3. Create a chromium directory for the checkout and change to it
```
$ mkdir ~/chromium && cd ~/chromium
```
4. Run the fetch tool from depot_tools to check out the code and its dependencies.
```
$ fetch --nohooks chromium
```
5. Check out version 58 of target chromium.
```
$ git checkout tags/58.0.3003.0 -b 58
```
6. Check out a version of depot_tools from around the same time as the target revision.
```
# Get date of current revision:
~/chromium/src $ COMMIT_DATE=$(git log -n 1 --pretty=format:%ci)

# Check out depot_tools revision from the same time:
~/depot_tools $ git checkout $(git rev-list -n 1 --before="$COMMIT_DATE" main)
~/depot_tools $ export DEPOT_TOOLS_UPDATE=0
```
7. Checkout all the submodules at their branch DEPS revisions.
```
$ gclient sync -D --force --reset --with_branch_heads
```
8. To create a build directory, run:
```
gn args out/ASan--
```
9. Set build arguments.
```
is_clang = true
clang_base_path = "../../../../../../llvm-4.0.0-project/ASan--Build"
is_asan = true
is_debug = ture
symbol_level = 1
is_component_build = true
pdf_use_skia=true
```
10. Build Chromium (the “chrome” target) with Ninja using the command:
```
ninja -C out/ASan-- chrome
```
## Performance Benchmarks
1. To run time-based benchmarks:
- Sunspider:
```
$ ./chrome https://webkit.org/perf/sunspider-0.9.1/sunspider-0.9.1/driver.html
```
- Kraken:
```
$ ./chrome https://mozilla.github.io/krakenbenchmark.mozilla.org/index.html
```
- Lite Brite(Please select "All" before running)
```
$ ./chrome https://testdrive-archive.azurewebsites.net/Performance/LiteBrite/
```

2. To run score-based benchmarks:
- Octane:
```
$ ./chrome https://chromium.github.io/octane/
```
- Basemark
```
$ ./chrome https://web.basemark.com/
```
- WebXPRT
```
$ ./chrome https://www.principledtechnologies.com/benchmarkxprt/webxprt/run-webxprt-mobile
```

## Chromium Bugs Reproduce
- To reproduce issue 848914:
```
$ ./chrome --disable-gpu ./Issue_848914_PoC/gpu_freeids.html
```

- To reproduce issue 1116869:
```
$ ./chrome ./Issue_1116869_PoC/poc_heap_buffer_overflow_1116869
```

- To reproduce issue 1099446:
```
$ ./chrome ./Issue_1099446_PoC/poc_heap_buffer_overflow_1099446
```

