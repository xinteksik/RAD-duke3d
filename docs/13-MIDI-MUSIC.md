# MIDI music candidate — 0.0.3-music01

Later update: the user confirmed that music plays on RAD and requested stereo/CFG
as in Wolf. The current SD now contains [0.0.4-stereo01](14-STEREO-AND-SID-CONFIG.md).
The following records the original music01 preparation and pre-hardware checks.

Music is implemented and verified on the host; a complete ARM64 firmware and
private SD payload are ready for a physical SID listening test. This candidate
has **not** been tested on RAD hardware. Sound effects remain disabled.

Kernel: 1,227,368 bytes, SHA-256
`548aa4e8ae8a437f12e9a1918dac2e128aaf17087de04eac9b7d55de69f0788a`.
Previous user-tested silent kernel and payload are preserved in
`build/sd-backups/0.0.2-controls01/`. The v0.1.0-alpha.1 release archives remain
unchanged and still contain that silent kernel.

## Private MIDI extraction

`tools/extract-midi.py` validates the exact shareware **1.3D** GRP hash, directory
bounds, MIDI header and track chunks, then writes identical private copies to
`build/music-assets/MIDI/`. Original GRP bytes are not changed. Re-running refuses
different existing output. Inventory and individual hashes are in
`references/MIDI-INVENTORY.json`.

| MIDI | Bytes | SMF tracks | Last parsed event, ms | Note-on events |
|---|---:|---:|---:|---:|
| GRABBAG | 14,287 | 13 | 62,088 | 4,326 |
| STALKER | 34,703 | 19 | 163,389 | 9,490 |
| DETHTOLL | 7,185 | 12 | 240,595 | 1,524 |
| STREETS | 52,051 | 9 | 357,995 | 10,718 |
| WATRWLD1 | 25,371 | 9 | 352,396 | 5,946 |
| SNAKE1 | 36,088 | 9 | 352,495 | 4,716 |
| THECALL | 16,764 | 9 | 375,000 | 4,128 |

All seven are SMF format 1. GRABBAG is the menu theme; STALKER is selected for
E1L1 by the original CON mapping. Playback reads MIDI through Duke's GRP file
layer. Extracted `SD/RADDUKE/MIDI/` files are for inspection/offline testing and
are not required at runtime. Original local shareware **1.1** is still unsupported.

## Implementation and licenses

`radduke_music.c` replaces the selected JFAudioLib MUSIC implementation with a
cooperative TinySoundFont/TinyMidiLoader backend. Existing MUSIC ABI names remain
for the game; new adapter symbols use radduke_. It handles track replacement,
program changes, notes, controllers, pitch bend, channel-10 percussion, volume,
pause/resume, stop, play-once and looping. It does not enable the FX mixer.

The menu patch starts its normal theme without reinstating Logo/demo playback.
The game selects level tracks as before. Non-MIDI waveform alternatives are
disabled in this candidate. Missing/invalid SoundFont falls back to silence
without preventing boot. MIDI data length/read errors are handled before parsing.

Unmodified TinySoundFont v0.9 (MIT) and TinyMidiLoader v0.7 (zlib) were copied from
the independent frozen Wolf snapshot. Its retained notice identifies upstream
revision `853a0a171759f1ddba0de1442133a75912bbeffa`. Exact copied bytes and origins
are in `references/MUSIC-DEPENDENCIES.json`; full notices stay in their headers
and in SD/LICENSES. The inherited 95-tap FIR remains GPL-3.0-or-later, with only
project symbol/header renames. These component grants do not resolve the
combined Build/GPL distribution issue documented in [the license review](06-LICENSE-REVIEW.md).

The private SoundFont copy comes from the sibling Wolf resources, read only;
no Wolf file was changed. Local file: `build/music-assets/soundfont.sf2`,
3,299,098 bytes, SHA-256
`a9058609b07c1f0e0c1f420e76c2e80ba78136e6173e836caab51ffb13e552f1`.
No redistribution permission is asserted for it. GRP, extracted MIDI, SF2, raw
PCM and previews remain private and must not be added to public packages.

## Audio transport

Synthesis runs on the main thread at 44,100 Hz, stereo, at most 64 voices and
-9 dB synth gain. The retained FIR decimates to 22,050 Hz. The cooperative ring
producer maintains 100 ms lookahead in the existing 16,384-sample ring and uses
the existing SID/Mahoney LUTs. Mono averages left/right; the existing secondary
SID state is honored if already configured. No new SID address detection or
configuration is introduced by this candidate.

The ring is serviced during explicit polling/waits and before/after presentation,
with synthesis outside the IRQ and scanner critical section. The initial epoch
starts after prefill. Kernel sample IRQ, DMA/PAL transfer, GPIO, clock/SD settings,
SID timing and frozen low-level sources remain unchanged. A long blocking load
can exhaust the lookahead; boot.log records underrun counts at powers of two.
Track stop/pause/volume changes can have queued-audio latency. Loops use the last
parsed event boundary and reset voices/controllers/filter; seamless release-tail
quality has not been established on hardware.

## Verification

- All seven actual MIDI files synthesized nonzero PCM. Tests traversed a full
  loop and play-once endpoint for every track, pause/resume, mute/stop and
  byte-identical output with different render chunk sizes.
- SID ring tests checked prefill epoch, mono/stereo LUT conversion, no premature
  overwrite, long-load underrun recovery and wraparound using a simulated clock.
- Actual host engine played GRABBAG in the menu and switched to STALKER in E1L1.
  Queued stereo PCM was captured through an SDL dummy sink: nonzero output with
  the font and exactly silent output without it. This is not physical listening.
- All twelve shared raw gameplay-control checks passed with the music backend.
- The complete firmware links all 56 core units with no undefined ELF symbols.
  Build log, sources, dependency hashes and a NOT_TESTED hardware record are saved.

Evidence: `references/music-candidate-03/`. Private audio previews:
`build/music-host-runs/menu/private-preview.wav` and
`build/music-host-runs/gameplay/private-preview.wav`.

```sh
python3 tools/extract-midi.py
python3 tools/prepare-music-assets.py
python3 tools/prepare-music-source.py
bash tools/test-music.sh
RADDUKE_MUSIC=1 python3 tools/build-menu-core.py host
bash tools/run-music-host.sh
python3 tools/verify-music-host.py
bash tools/build-music-firmware.sh
```

The first-time SD staging script requires the previous verified controls payload
and refuses an existing backup. It has already been run for this candidate;
do not rerun blindly. Host run directories likewise refuse overwrite. All work
stays here. No physical SD card was written or ejected.

For the hardware test, use the prepared private SD payload after backing up the
target card. Listen for the menu theme and its level change, and report the
kernel hash, SID model, duration, dropouts/distortion and gameplay responsiveness.
Keep `RADDUKE/boot.log` and `RADDUKE/duke3d.log`. Hardware performance and sound
quality remain unverified until that test; preserve the silent fallback payload.
