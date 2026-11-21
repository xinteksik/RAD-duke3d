#!/usr/bin/env bash
set -euo pipefail
actual=$(cd -- "$(dirname -- "$0")/.." && pwd)
ln -sfn "$actual" /tmp/radduke-work
repo=/tmp/radduke-work
export PATH="$repo/toolchains/gcc-arm-10.2-2020.11-x86_64-aarch64-none-elf/bin:$PATH"
export LC_ALL=C TZ=UTC SOURCE_DATE_EPOCH=1795171200 RADDUKE_MUSIC=1 RADDUKE_STEREO=1 RADDUKE_EFFECTS=1
stdlib="$repo/upstream/rad-baseline/upstream/circle-stdlib"
cmp "$repo/upstream/rad-baseline/overrides/Circle/include/circle/sysconfig.h" "$stdlib/libs/circle/include/circle/sysconfig.h"
python3 "$repo/tools/build-menu-core.py" arm64
out="$repo/build/effects-firmware"
mkdir -p "$out"
cp "$repo/tools/firmware.Makefile" "$out/Makefile"
make -C "$out" REPO="$repo" CORE="$repo/build/effects-arm64" 'RADDUKE_EXTRA_FLAGS=-DRADDUKE_MUSIC -DRADDUKE_STEREO -DRADDUKE_EFFECTS' -j2 > "$repo/build/effects-firmware.log" 2>&1
sha256sum "$out/kernel8.img" | tee "$repo/build/effects-firmware.sha256"
aarch64-none-elf-nm -u "$out/kernel8.elf" > "$repo/build/effects-firmware-imports.txt"
echo EFFECTS_FIRMWARE_LINK_OK
