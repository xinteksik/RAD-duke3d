"""Fetch exact release dependencies; never consult a sibling project or an SD device."""
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys

root = Path(__file__).resolve().parents[1]
lock = json.loads((root / 'references/ALPHA-UPSTREAM.lock.json').read_text())
verify_only = '--verify-snapshot' in sys.argv
for entry in lock['repositories']:
    dest = root / entry['path']
    if verify_only:
        assert dest.is_dir(), str(dest)
        continue
    if not dest.exists():
        subprocess.run(['git', 'clone', '--no-checkout', entry['url'], str(dest)], check=True)
        subprocess.run(['git', '-C', str(dest), 'checkout', '--detach', entry['revision']], check=True)
    revision = subprocess.check_output(['git', '-C', str(dest), 'rev-parse', 'HEAD'], text=True).strip()
    assert revision == entry['revision'], (dest, revision)

for entry in lock['overrides']:
    src, dest = root / entry['source'], root / entry['target']
    assert hashlib.sha256(src.read_bytes()).hexdigest() == entry['sha256'], str(src)
    if not verify_only:
        dest.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(src, dest)
    assert dest.read_bytes() == src.read_bytes(), str(dest)

for src in (root / 'vendor/rad-wolf-headers').glob('*.h'):
    dest = root / 'upstream/rad-baseline/src' / src.name
    if not verify_only:
        dest.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(src, dest)
    assert dest.read_bytes() == src.read_bytes(), str(dest)

# Frozen comparison path used by the runtime wrapper.
for entry in lock['overrides']:
    dest = root / 'upstream/rad-baseline' / entry['source'].removeprefix('vendor/')
    dest.parent.mkdir(parents=True, exist_ok=True)
    if not verify_only:
        shutil.copyfile(root / entry['source'], dest)
    assert dest.read_bytes() == (root / entry['source']).read_bytes(), str(dest)

# configure, makeall and config.guess must remain executable after ZIP extraction.
for base in ('upstream/rad-baseline/upstream/circle-stdlib',):
    for p in (root / base).rglob('*'):
        if p.is_file() and (p.name in ('configure', 'makeall', 'config.guess', 'config.sub', 'install-sh', 'mkinstalldirs') or p.suffix == '.sh'):
            p.chmod(p.stat().st_mode | 0o111)
manifest = root / 'references/ALPHA-SOURCE-SHA256SUMS'
if manifest.exists():
    count = 0
    for line in manifest.read_text().splitlines():
        digest, name = line.split('  ', 1)
        assert hashlib.sha256((root / name).read_bytes()).hexdigest() == digest, name
        count += 1
    print('SOURCE_SNAPSHOT_VERIFIED', count)
print('PINNED_DEPENDENCIES_READY')
