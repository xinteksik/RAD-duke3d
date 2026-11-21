#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "$0")/.."
out=build/effects-tests
mkdir -p "$out"
gcc -std=c99 -O2 -Isrc tests/test_pcm.c src/radduke_pcm.c -o "$out/pcm"
"$out/pcm" | tee "$out/pcm.log"
python3 - <<'PY'
import subprocess
from pathlib import Path
root=Path.cwd();source=root/'build/effects-source';out=root/'build/effects-tests'
files=['asssys.c','drivers.c','driver_nosound.c','fx_man.c','multivoc.c','mix.c','mixst.c','pitch.c','vorbis.c']
cmd=['gcc','-std=c99','-O2','-g','-Werror=implicit-function-declaration','-DUSE_ASM=0','-Isrc',
     '-I'+str(source/'jfaudiolib/include'),'-I'+str(source/'jfaudiolib/src'),
     'tests/test_multivoc.c','src/radduke_pcm.c',*[str(source/'jfaudiolib/src'/p) for p in files],'-lm','-o',str(out/'multivoc')]
subprocess.run(cmd,check=True)
PY
"$out/multivoc" build/effects-assets/*.VOC > "$out/multivoc.log" 2>&1
cat "$out/multivoc.log"
echo EFFECTS_UNIT_TESTS_PASSED
