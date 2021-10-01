# Chromium TestCases

## Performance Benchmarks
1. To run time-based benchmarks:
- Sunspider:
```
$ /<ASan|ASan-->/chrome https://webkit.org/perf/sunspider-0.9.1/sunspider-0.9.1/driver.html
```
- Kraken:
```
$ /<ASan|ASan-->/chrome https://mozilla.github.io/krakenbenchmark.mozilla.org/index.html
```
- Lite Brite(Please select "All" before running)
```
$ /<ASan|ASan-->/chrome https://testdrive-archive.azurewebsites.net/Performance/LiteBrite/
```

2. To run score-based benchmarks:
- Octane:
```
$ /<ASan|ASan-->/chrome https://chromium.github.io/octane/
```
- Basemark
```
$ /<ASan|ASan-->/chrome https://web.basemark.com/
```
- WebXPRT
```
$ /<ASan|ASan-->/chrome https://www.principledtechnologies.com/benchmarkxprt/webxprt/run-webxprt-mobile
```

## Chromium Bugs Reproduce
To reproduce issue 848914:
```
$ /<ASan|ASan-->/chrome --disable-gpu Issue_848914_PoC/gpu_freeids.html
```

