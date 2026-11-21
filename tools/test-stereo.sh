#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "$0")/.."
out=build/stereo-tests
mkdir -p "$out"
gcc -std=c99 -O2 -Isrc tests/test_audio_config.c src/radduke_audio_config.c -o "$out/config"
"$out/config" | tee "$out/config.log"
gcc -std=c99 -O2 -Isrc -Iupstream/rad-baseline/src tests/test_audio_startup.c src/radduke_audio_config.c src/radduke_audio_startup.c -o "$out/startup"
"$out/startup" "$out/temporary-audio.cfg" | tee "$out/startup.log"
gcc -std=c99 -O2 -DRADDUKE_STEREO -Isrc -Iupstream/rad-baseline/src tests/test_music_ring.c src/radduke_music_ring.c -o "$out/ring"
"$out/ring" | tee "$out/ring.log"
gcc -std=c99 -O2 -Isrc -Iupstream/jfduke3d/jfaudiolib/include tests/test_music_stereo.c src/radduke_music.c src/radduke_downsample.c -lm -o "$out/pan"
RADDUKE_SOUNDFONT="$PWD/build/music-assets/soundfont.sf2" "$out/pan" > "$out/pan.log" 2>&1
cat "$out/pan.log"
echo STEREO_TESTS_PASSED
