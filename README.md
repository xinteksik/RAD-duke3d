# RAD DUKE3D v0.1.0-alpha.2

First boot video:

[![Watch the video on YouTube](https://img.youtube.com/vi/s_PbXhxlN8E/maxresdefault.jpg)](https://youtu.be/s_PbXhxlN8E)

Experimental Duke Nukem 3D for Pi Zero 2 W, RAD Expansion Unit and PAL C64.
Candidate `0.0.6-weapons01` adds weapon selection on digits **1-0** to MIDI music,
original MIDI panorama and sound effects. Software rendering: 320 x 200.

Kernel SHA-256: `c00374301d26d59e4e2677c7b7d6dc975f7279e97a71de728dff16a4b60f6454`. Kernel bytes: 1232240.
Full ARM64 link and host tests passed. This candidate has **not** been tested on
physical hardware. Earlier music/stereo reports do not establish effects or
weapon selection on this candidate. No measured FPS or map completion is claimed.

## Controls and private data

IJKL move/turn, Return confirm, Commodore/F fire, A jump, either Shift run,
Space use, Z crouch, back-arrow Escape, Y/N confirm, 1-0 select owned weapons.
Shareware supports slots 1-6; 7-0 retain the original edition restrictions.
No weapon ownership or game edition check was changed.

Add your own lawful **shareware 1.3D** `duke3d.grp` under `RADDUKE/` in the SD
package. Local shareware 1.1 is unsupported. Add your own licensed `soundfont.sf2`
for music; effects work without it. MIDI is read from the supplied GRP.
Game data, MIDI/VOC, SoundFonts, saves, DOS programs and captures are excluded.
`RADDUKE/AUDIO.CFG` defaults to SID1 $D400/8580, SID2 $D500/6581, stereo ON,
MIDI_STEREO_WIDTH=0. Adjust models and addresses to actual hardware before use.

## Build from the full source archive

Linux/WSL needs Python 3, GCC/G++, make, git, ripgrep and SDL2 development files
for host diagnostics. Install the exact AArch64 GNU 10.2-2020.11 toolchain under
`toolchains/gcc-arm-10.2-2020.11-x86_64-aarch64-none-elf/`; its archive SHA-256 and
dependency pins are in `references/ALPHA-UPSTREAM.lock.json`.

```sh
python3 tools/bootstrap-alpha.py --verify-snapshot
python3 tools/prepare-menu-source.py
python3 tools/prepare-music-source.py
python3 tools/prepare-effects-source.py
bash tools/test-alpha.sh
bash tools/build-menu-runtime.sh
bash tools/build-effects-firmware.sh
```

Output: `build/effects-firmware/kernel8.img`. The smaller GitHub overlay ZIP omits
dependency sources; run `python3 tools/bootstrap-alpha.py` to fetch exact pins.
Host weapon/effect fixtures require private game data and are not included.
The full ARM64 build was run in the working tree; a separate complete runtime
rebuild from the exported archive was not run. See `VERIFICATION.json`.

## Publication status and notices

See [LICENSE.md](LICENSE.md) for the component license index.

Prepared locally for review; **not uploaded**. Combined Build/GPL distribution
remains unresolved; see [the revision-specific review](docs/06-LICENSE-REVIEW.md).
This package does not grant publication rights. No aggregate license is assigned.
Original component notices are preserved in LICENSES, vendor and source headers.

"Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman.
Ken Silverman's official web site: http://www.advsys.net/ken
See LICENSES/BUILDLIC.TXT for license info.
