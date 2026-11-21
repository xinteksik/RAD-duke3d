"""Normalize generated Newlib paths to a per-process alias; no source changes."""
from pathlib import Path
root = Path(__file__).resolve().parents[1]
build = root / 'upstream/rad-baseline/upstream/circle-stdlib/build/circle-newlib'
old = str(root).encode()
new = b'/tmp/radduke-work'
changed = []
for p in build.rglob('*'):
    if p.is_file() and p.name in ('Makefile', 'config.status'):
        b = p.read_bytes()
        normalized = b.replace(old, new)
        if p.name == 'Makefile':
            # NTFS sandbox permits removing generated members, but removal
            # of their open staging directory fails. Reuse that directory;
            # still fail if deleting its old object files fails.
            normalized = normalized.replace(b'rm -rf tmp', b'rm -f tmp/*.o')
            normalized = normalized.replace(b'mkdir tmp', b'mkdir -p tmp')
        if normalized != b:
            p.write_bytes(normalized)
            changed.append(p.relative_to(build).as_posix())
(root / 'build/newlib-path-normalization.log').write_text('\n'.join(changed) + '\n')
print('Normalized', len(changed), 'generated files')
