# Weapon selection and v0.1.0-alpha.2

Candidate **0.0.6-weapons01** completes digit-key weapon selection on top of
0.0.5-audio01 music, original MIDI panorama and effects. The new kernel is
packaged separately from the user-tested silent v0.1.0-alpha.1.

| Digit | Original weapon slot |
|---|---|
| 1 | Mighty Foot / kick |
| 2 | Pistol |
| 3 | Shotgun |
| 4 | Chaingun |
| 5 | RPG |
| 6 | Pipebomb |
| 7 | Shrinker |
| 8 | Devastator |
| 9 | Tripbomb |
| 0 | Freezethrower |

The raw C64 scanner already supplies ASCII digits. The adapter now maps these
to the original engine scancodes 0x02-0x0b and retains ASCII menu input.
Numeric ASCII values are distinct from raw Shift values 1/6 and Commodore 4.
Held/released states use the existing input boundary; PAL/DMA/GPIO/SD/SID
transport and timer code were not changed.

The game keeps ownership, ammo and edition rules. In the tested shareware 1.3D,
only slots 1-6 are available. Slots 7-0 are mapped by the adapter but deliberately
produce no weapon request in the original shareware game. Registered data was
not tested. A key press during a weapon animation may be ignored by the game.

## Verification

Platform unit tests cover all ten digit scancodes, releases, Shift independence,
alias/FIFO releases and the 120 Hz timer wrap. FatFs path tests passed.
The real E1L1 host baseline switches from pistol to kick with 1 and back with 2;
unowned slots 3-6 are rejected. A separate **host-only fixture** grants the first
six slots and ammo and confirms six actual weapon changes. It is never compiled
into the ARM64 firmware. Both runs confirm release and shareware 7-0 rejection.

An earlier diagnostic pressed 1 during the initial pistol animation and did not
establish a switch. The final probe waits 2.5 seconds; original game rules were
not altered. Earlier diagnostics remain in ignored build output.
Evidence: `references/weapons-candidate-06/`.

```sh
RADDUKE_MUSIC=1 RADDUKE_STEREO=1 RADDUKE_EFFECTS=1 python3 tools/build-menu-core.py host
RADDUKE_WEAPON_RUN=build/weapons-host-final bash tools/run-weapons-host.sh
RADDUKE_WEAPON_FIXTURE=1 RADDUKE_WEAPON_RUN=build/weapons-host-owned bash tools/run-weapons-host.sh
python3 tools/verify-weapons-host.py
bash tools/test-alpha.sh
bash tools/build-effects-firmware.sh
```

Run scripts refuse existing evidence directories. Private game data and a
licensed SoundFont are required for these host runs. Decoder/PCM and stereo
regressions were also run before packaging.

## Delivery and hardware scope

`releases/v0.1.0-alpha.2/` contains a GitHub source overlay ZIP, data-free SD ZIP,
full source tar, release notes, verification metadata and SHA-256 checksums.
The complete 59-unit ARM64 core and firmware link passed with no undefined ELF
symbols. Dependency source files are verified against the earlier frozen export.
A separate full runtime rebuild from the exported tar was not performed.

Private `SD/` is updated only after preserving its complete previous payload at
`build/sd-backups/0.0.5-audio01/`. Physical media are not accessed or ejected.
Physical digit selection, effects, balance, duration and performance are still
**NOT TESTED** for this candidate. Earlier music/stereo reports remain separate.

Public archives exclude GRP/CON/ART/MAP/RTS, MIDI/VOC, SoundFonts, saves, DOS
programs and private captures. Original component notices remain intact.
The existing combined Build/GPL distribution issue is unresolved; preparation
does not clear public distribution. No GitHub upload is performed.
