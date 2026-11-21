"""Compile the shared menu adapter and pinned core for host or Circle."""
import concurrent.futures
import hashlib
import json
from pathlib import Path
import os
import subprocess
import sys

root = Path(__file__).resolve().parents[1]
target = sys.argv[1] if len(sys.argv) > 1 else 'host'
assert target in ('host', 'arm64')
music = os.environ.get('RADDUKE_MUSIC') == '1'
stereo = music and os.environ.get('RADDUKE_STEREO') == '1'
effects = stereo and os.environ.get('RADDUKE_EFFECTS') == '1'
out = root / 'build' / (('effects-' if effects else 'stereo-' if stereo else 'music-' if music else 'menu-') + target)
out.mkdir(parents=True, exist_ok=True)
source = root / ('build/effects-source' if effects else 'build/music-source' if music else 'build/menu-source')
core_sources = json.loads((root / 'references/CORE-SOURCES.json').read_text())
files = [(source / name, name.replace('/', '_')) for name in core_sources]
if music:
    files = [(p, n) for p, n in files if n != 'jfaudiolib_src_music.c']
    files += [(root / 'src' / n, n) for n in ('radduke_music.c', 'radduke_downsample.c')]
    if target == 'arm64': files += [(root / 'src/radduke_music_ring.c', 'radduke_music_ring.c')]
if stereo:
    files += [(root / 'src/radduke_audio_config.c', 'radduke_audio_config.c')]
    if target == 'arm64': files += [(root / 'src/radduke_audio_startup.c', 'radduke_audio_startup.c')]
if effects:files += [(root / 'src/radduke_pcm.c','radduke_pcm.c')]
files += [(root / 'src' / name, name) for name in ('radduke_build_platform.c', 'radduke_pathstat.c')]
if target == 'host':
    cc = 'gcc'
    flags = ['-std=c99', '-O2', '-g', '-fno-strict-aliasing', '-DRADDUKE_PLATFORM', '-DUSE_OPENGL=0', '-DUSE_POLYMOST=0', '-DUSE_ASM=0', '-Werror=implicit-function-declaration']
    files += [(root / 'tools/radduke_host_driver.c', 'radduke_host_driver.c')]
    files += [(root / 'tools/radduke_gameplay_probe.c', 'radduke_gameplay_probe.c')]
    files += [(root / 'tools/radduke_weapon_probe.c', 'radduke_weapon_probe.c')]
    sdlflags = subprocess.check_output(['sdl2-config', '--cflags'], text=True).split()
else:
    cc = str(root / 'toolchains/gcc-arm-10.2-2020.11-x86_64-aarch64-none-elf/bin/aarch64-none-elf-gcc')
    flags = ['-std=c99', '-O2', '-g', '-fno-strict-aliasing', '-ffreestanding', '-mcpu=cortex-a53', '-mlittle-endian', '-DRADDUKE_PLATFORM', '-DAARCH=64', '-DUSE_OPENGL=0', '-DUSE_POLYMOST=0', '-DUSE_ASM=0', '-Werror=implicit-function-declaration']
    stdlib = root / 'upstream/rad-baseline/upstream/circle-stdlib'
    flags += ['-I' + str(stdlib / p) for p in ('install/aarch64-none-circle/include', 'build/circle-newlib/aarch64-none-circle/newlib/targ-include', 'libs/circle-newlib/newlib/libc/include', 'include', 'libs/circle/include', 'libs/circle/addon')]
    sdlflags = []
flags += ['-I' + str(root / 'src')]
flags += ['-fsigned-char']
if music:
    flags += ['-DRADDUKE_MUSIC']
    if target == 'arm64': flags += ['-I' + str(root / 'upstream/rad-baseline/src')]
if stereo: flags += ['-DRADDUKE_STEREO']
if effects: flags += ['-DRADDUKE_EFFECTS']
flags += ['-I' + str(source / p) for p in ('src', 'jfbuild/include', 'jfbuild/src', 'jfmact', 'jfaudiolib/include', 'jfaudiolib/src', 'jfbuild/libsquish')]
# Version declarations are generated explicitly without relying on OS or Git.
stamp = out / 'versions.c'
stamp.write_text('const char *game_version="RADDUKE-55c5f95"; const char *build_version="40a98be";\nconst char *game_date="2026-11-20", *game_time="00:00:00", *build_date="2026-11-20", *build_time="00:00:00";\n')
files += [(stamp, 'versions.c')]
header_digest = hashlib.sha256()
for folder in (root / 'src', root / 'third_party', source):
    for header in sorted(folder.rglob('*.h')):
        header_digest.update(str(header.relative_to(root)).encode())
        header_digest.update(header.read_bytes())
shared_headers = header_digest.digest()
def compile_one(item):
    path, name = item
    obj = out / (name + '.o')
    # Recompile when source, flags or any adapter header changes.
    signature = hashlib.sha256(path.read_bytes() + repr(flags).encode() + shared_headers).hexdigest()
    sigpath = obj.with_suffix('.signature')
    if obj.exists() and sigpath.exists() and sigpath.read_text() == signature:
        return {'source': str(path), 'object': str(obj), 'exit_code': 0, 'cached': True, 'diagnostics': ''}
    cmd = [cc, *flags, *(sdlflags if name == 'radduke_host_driver.c' else []), '-c', str(path), '-o', str(obj)]
    p = subprocess.run(cmd, text=True, capture_output=True)
    if p.returncode == 0: sigpath.write_text(signature)
    return {'source': str(path), 'object': str(obj), 'command': cmd, 'exit_code': p.returncode, 'diagnostics': p.stdout + p.stderr}
with concurrent.futures.ThreadPoolExecutor(max_workers=2) as pool:
    results = list(pool.map(compile_one, files))
(out / 'build.log').write_text('\n'.join(r['source'] + '\n' + r['diagnostics'] for r in results))
(out / 'RESULTS.json').write_text(json.dumps(results, indent=2) + '\n')
failed = [r for r in results if r['exit_code']]
for r in failed: print(r['source'], r['diagnostics'][:3000])
if failed: sys.exit(1)
if target == 'host':
    libs = subprocess.check_output(['sdl2-config', '--libs'], text=True).split()
    cmd = [cc, '-o', str(out / 'radduke-host'), *[r['object'] for r in results], '-lm', *libs]
    p = subprocess.run(cmd, text=True, capture_output=True)
    (out / 'link.log').write_text(p.stdout + p.stderr)
    print(p.stdout + p.stderr)
    if p.returncode: sys.exit(p.returncode)
    print('HOST_ADAPTER_BUILD_OK', hashlib.sha256((out / 'radduke-host').read_bytes()).hexdigest())
else:
    print('ARM64_MUSIC_CORE_OK' if music else 'ARM64_MENU_CORE_OK', len(results))
