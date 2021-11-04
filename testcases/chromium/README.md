# Chromium Test Cases
- Please note that our scripts are designed for building Chromium in Docker.
## Build Chromium with ASan--
1. Run the script to download Chromium source code and install dependencies:
```
bash build_chromium.sh
```
2. To create a build directory, run:
```
cd chromium/src
export PATH="$PATH:/home/testcases/chromium/depot_tools"
gn args out/ASan--
```
3. Set build arguments. Here is an example to build with original ASan. To build with ASan--, please using `clang_base_path = "/home/llvm-4.0.0-project/ASan--Build"`:
```
is_clang = true
clang_base_path = "/home/original_llvm/ASan--Build"
is_asan = true
is_debug = true
symbol_level = 1
is_component_build = true
pdf_use_skia=true
clang_use_chrome_plugins = false
```
4. Build Chromium (the “chrome” target) with Ninja using the command:
```
ninja -C out/ASan-- chrome
```
## Performance Benchmarks
1. To run time-based benchmarks:
```
export ASAN_OPTIONS=halt_on_error=0:detect_odr_violation=0:detect_container_overflow=0
```
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
```
export ASAN_OPTIONS=halt_on_error=0:detect_odr_violation=0:detect_container_overflow=0
```
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

