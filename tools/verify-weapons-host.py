"""Check real game weapon changes, ownership rejection and shareware restrictions."""
import csv
import json
import shutil
from pathlib import Path

root = Path(__file__).resolve().parents[1]
out = root / 'references/weapons-candidate-06'
out.mkdir(exist_ok=True)
for mode, directory in [('baseline', 'weapons-host-final'), ('owned-fixture', 'weapons-host-owned')]:
    source = root / 'build' / directory
    rows = list(csv.DictReader((source / 'weapons.csv').open()))
    assert len(rows) == 20
    for i, digit in enumerate('1234567890'):
        held, settled = rows[2*i:2*i+2]
        assert held['digit'] == settled['digit'] == digit
        assert int(held['shareware']) == 1
        assert int(held['request']) == (i+1 if i < 6 else 0)
        assert int(settled['request']) == 0
        expected = i if mode == 'owned-fixture' and i < 6 else (0 if i == 0 else 1)
        if mode == 'owned-fixture' and i >= 6:
            expected = 5
        assert int(settled['weapon']) == expected, (mode, digit, settled)
        if mode == 'baseline' and 2 <= i < 6:
            assert int(settled['owned']) == 0
    for filename in ('weapons.csv', 'host.log'):
        target = out / mode / filename
        target.parent.mkdir(exist_ok=True)
        shutil.copyfile(source / filename, target)
result = {'candidate': '0.0.6-weapons01', 'digit_mappings_checked': 10,
          'baseline_kick_and_pistol_switch': True, 'unowned_weapons_rejected': True,
          'owned_fixture_actual_weapon_changes': 6, 'shareware_7_to_0_rejected': True,
          'fixture_scope': 'Host-only grants slots 1-6 and ammo; never linked into firmware',
          'physical_keyboard_tested': False, 'registered_data_tested': False}
(out / 'HOST-RESULTS.json').write_text(json.dumps(result, indent=2)+'\n')
print(json.dumps(result, indent=2))
