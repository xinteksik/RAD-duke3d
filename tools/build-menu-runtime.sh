#!/usr/bin/env bash
# Build only the copied frozen runtime. No removable-device operations.
set -euo pipefail
physical=$(cd -- "$(dirname -- "$0")/.." && pwd -P)
ln -sfn "$physical" /tmp/radduke-work
repo=/tmp/radduke-work
mkdir -p "$repo/build"
stdlib="$repo/upstream/rad-baseline/upstream/circle-stdlib"
export RADDUKE_PHYSICAL_ROOT="$physical"
export PWDCMD="/bin/sh $repo/tools/pwd-alias.sh"
export PATH="$repo/toolchains/gcc-arm-10.2-2020.11-x86_64-aarch64-none-elf/bin:$PATH"
export LC_ALL=C TZ=UTC SOURCE_DATE_EPOCH=1795171200
cmp "$repo/upstream/rad-baseline/overrides/Circle/include/circle/sysconfig.h" "$stdlib/libs/circle/include/circle/sysconfig.h"
if [[ ! -f "$stdlib/Config.mk" ]]; then
    (cd "$stdlib" && ./configure --raspberrypi 3 --prefix aarch64-none-elf-) > "$repo/build/runtime-configure.log" 2>&1
fi
if [[ ! -f "$stdlib/libs/circle/lib/libcircle.a" || ! -f "$stdlib/libs/circle/addon/SDCard/libsdcard.a" || ! -f "$stdlib/libs/circle/addon/fatfs/libfatfs.a" ]]; then
    make -C "$stdlib" circle -j2 > "$repo/build/runtime-circle.log" 2>&1
fi
if [[ -f "$stdlib/install/aarch64-none-circle/lib/libcirclenewlib.a" && -f "$stdlib/install/aarch64-none-circle/lib/libm.a" && -f "$stdlib/install/aarch64-none-circle/lib/libc.a" ]]; then
    echo FROZEN_RUNTIME_READY
    exit 0
fi
for attempt in 1 2 3; do
    python3 "$repo/tools/fix-newlib-generated-paths.py"
    log="$repo/build/runtime-newlib-$attempt.log"
    if make -C "$stdlib" newlib -j2 > "$log" 2>&1; then
        echo FROZEN_RUNTIME_BUILT
        exit 0
    fi
    # Only recover the already diagnosed generated-path/temp-dir failures.
    if ! rg -q "cannot remove 'tmp'|error: Projects/RAD|error: DUKE3D/" "$log"; then
        tail -n 35 "$log"
        exit 1
    fi
done
exit 1
