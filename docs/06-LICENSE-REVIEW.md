# License review of the pinned revisions

Private host and bare-metal builds exist. Public distribution of the linked
firmware has **not been cleared**. This is a record of concrete grants and open
questions, not a legal certification. No aggregate project license is assigned.
Pins are in `references/JFDUKE3D-HOST.lock.json` and the RAD dependency lock.
Full copied notices and their origins/hashes are in `references/license-evidence/`.

The later private music candidate adopts the frozen TinySoundFont/TinyMidiLoader
headers with their MIT/zlib grants and the GPLv3 FIR/ring implementation. Exact
hashes are in `references/MUSIC-DEPENDENCIES.json`. The SoundFont and extracted
game MIDI remain private; no permission to redistribute them is asserted.
See [the music candidate](13-MIDI-MUSIC.md). This addition does not clear the
existing Build/GPL combination for public distribution.

The later private 0.0.5-audio01 effects candidate activates the already-pinned
JFAudioLib MultiVoc/VOC mixer through project PCM glue. The decoder/mixer and
driver source notices remain GPL-2.0-or-later; new glue uses GPL-3.0-or-later.
No new codec/library was downloaded or aggregate license assigned. See
[the effects integration](15-EFFECTS-AND-ORIGINAL-PAN.md) and its source patch.

| Component | Grant and obligations found | Exceptions / unresolved scope |
|---|---|---|
| JFDuke3D game | game.c, grpscan.c and release notes state GPL-2.0-or-later; retain authors, GPL and corresponding source obligations | Documentation acknowledges Build's different license. No explicit exception covering the added GPLv3 RAD/Circle combination was located. Do not extend that acknowledgment to arbitrary new combinations. |
| JFBuild / C renderer | COPYING: Build Source Code License, 20 June 2000; derivatives distributed free over the Internet, without commercial use. Include BUILDLIC.TXT, Ken's notice/site and mark modified sources. LICENSE applies the same terms to Fowler's changes | Not GPL. The noncommercial/distribution restrictions obstruct ordinary GPL combined distribution. Separate folders or object files do not solve this. |
| KPLIB | kplib.c has its own Ken Silverman attribution/use notice | Keep its complete header; do not treat this grant as relicensing all of Build. |
| JFMACT | animlib.c, file_lib.c and mathutil.c have GPL-2.0-or-later notices and inherited Duke/MACT/Fowler credits | control.c, keyboard.c, mouse.c and scriplib.c also have shorter headers without full grants. No standalone aggregate LICENSE was found in this checkout. Clarify coverage before public distribution. |
| JFAudioLib | multivoc.c, mixer and SDL driver expressly use GPL-2.0-or-later; retain Apogee/Dose/Fowler notices | Host baseline includes mixer/MIDI interfaces, nosound and SDL. Direct ALSA/FluidSynth/Vorbis backends were not selected. The alpha skips audio initialization. |
| RAD/RAD-Doom | Revision 81752f4…; lowlevel_dma.cpp, rad_doom.cpp and rad_reu.cpp have Dachsbacher GPL-3.0-or-later headers; root GPLv3 retained | Other inherited files may have different grants. The adapter now retains selected hardware sources; do not infer a single license for the entire upstream tree. |
| Circle | Revision 75bc089…; LICENSE GPLv3 and inspected types.h GPL-3.0-or-later | No exception covering the new Build/GPL combination was located in the inspected material. Assess the objects actually linked. |
| circle-stdlib | Revision b517028…; README specifies GPLv3, full LICENSE retained | This is not the license of every libc object. |
| circle-newlib / Newlib | Revision 6fe4bb7…; COPYING.NEWLIB collects many per-file BSD/permissive grants and exceptions | Preserve selected libc/libm and Circle syscall-glue notices. The root COPYING is not a uniform license for all runtime objects; some glue has short/no file-specific grant headers. |
| Inherited Wolf adapters | GPL-3.0-or-later according to retained source notices | Their license does not authorize combining otherwise incompatible Build components. |
| TinySoundFont / TinyMidiLoader | Reference tsf.h v0.9 MIT; tml.h v0.7 zlib; retain their distinct credits/terms | Not linked in the Duke alpha. SoundFonts/MIDI have separate rights. |
| SDL2 host | Installed 2.30.0; mainly zlib plus per-component permissive notices | Dynamic host dependency; distribution copyright inventory retained. Not linked into the firmware. |
| Host libc/libm | glibc 2.39, mainly LGPL-2.1-or-later plus per-file grants | Dynamic system libraries; direct ELF NEEDED: SDL2, libc, libm. libstdc++ is not a direct dependency. System packages are not bundled. |

Primary pinned texts:
[JFBuild COPYING](https://github.com/jonof/jfbuild/blob/40a98be571e359a70fc60476899e4650e285dbca/COPYING),
[Fowler LICENSE](https://github.com/jonof/jfbuild/blob/40a98be571e359a70fc60476899e4650e285dbca/LICENSE),
[JFDuke3D release notes](https://github.com/jonof/jfduke3d/blob/55c5f9592d2a78e427a2bbdb13ce3c4c9ccf6f04/releasenotes.html).

GPLv2 sections 2/6 and GPLv3 sections 5/10 require preserving their freedoms and
not imposing additional restrictions. Build restricts commercial use and
distribution channels. Choosing GPLv3 under the game's "or later" clause does
not remove that conflict. Shipping source alongside the executable does not
resolve it either. A covering permission/exception from the necessary rights
holders, or a compatible replacement implementation, is required before treating
the combination as cleared. Changing a name or using another Build fork is not
enough. No authors have been contacted and no permission has been obtained here.

Game LICENSE.DOC/LICENSE.TXT have their own conditions on intact shareware
distribution; they do not automatically authorize extracting GRP/CON for this
port. The public-facing archive set excludes game data, music, SoundFonts, DOS
executables, saves and private captures. Keep legal notices verbatim, even when
translating project documentation. Reassess actual file headers, link maps and
new dependencies before expanding the firmware.
