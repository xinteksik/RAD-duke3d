#!/usr/bin/env bash
# Data-free platform and path regression tests. Optional host build needs SDL2.
set -euo pipefail
cd -- "$(dirname -- "$0")/.."
mkdir -p build/alpha-tests
gcc -std=c99 -O2 -ffunction-sections -fdata-sections -DRADDUKE_PLATFORM \
 -Isrc -Ibuild/menu-source/jfbuild/include -Ibuild/menu-source/jfbuild/src \
 tests/test_platform.c src/radduke_build_platform.c -Wl,--gc-sections -o build/alpha-tests/platform
build/alpha-tests/platform
g++ -std=c++11 -O2 -DRADDUKE_FS_PATH_TEST_ONLY -Isrc \
 tests/test_fs_path.cpp src/radduke_fs.cpp -o build/alpha-tests/path
build/alpha-tests/path
