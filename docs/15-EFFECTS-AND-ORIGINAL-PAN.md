# Music, original panorama and effects — 0.0.5-audio01

The user reported that music plays and stereo **seems** functional with the prior
stereo candidate. That qualified observation is preserved verbatim in
`references/hardware-tests/0.0.4-stereo01.json`; it is not a measured channel
separation, duration or independently read card hash.

The next private candidate, **0.0.5-audio01**, is staged in `SD/`. It adds sound
effects and makes the original MIDI panorama the default. The previous stereo
payload is preserved in `build/sd-backups/0.0.4-stereo01/`. Public alpha archives
are unchanged. No physical card was written or ejected.

Kernel: 1,232,176 bytes, SHA-256
`54e1485740b96a6d879c6f23af84ff9cec99c5f87e4d20cf423ec51f1247ab27`.
**Physical effects playback, balance and performance remain untested.**

## Original MIDI stereo

`RADDUKE/AUDIO.CFG` retains stereo and the previous SID addresses/types, but now
sets `MIDI_STEREO_WIDTH=0`. This disables extra automatic instrument placement;
it does not disable authored MIDI pan, SoundFont stereo or the two-SID output.
The optional Wolf spread remains available by choosing a nonzero width.

The actual original files contain MIDI Pan CC10/42 events. The read-only audit
in `references/MIDI-PAN-INVENTORY.json` found:

| MIDI | Pan events |
|---|---:|
| GRABBAG | 12 |
| STALKER | 18 |
| DETHTOLL | 11 |
| STREETS | 2,833 |
| WATRWLD1 | 1,303 |
| SNAKE1 | 7 |
| THECALL | 310 |

The fixed-length synth test renders all seven with width zero. GRABBAG and
STALKER retain clear left/right differences. Some authored openings are nearly
centered before later instruments or pan changes arrive; an early test wrongly
required every three-second opening to have a large stereo difference. That
expectation was corrected, with the failed diagnostic retained in ignored build
output. No MIDI, decoder or synth behavior was changed to satisfy that test.

## Effects implementation

`radduke_pcm.c` connects the pinned JFAudioLib/MultiVoc mixer to the existing
cooperative output. Its buffer-division/callback order follows the pinned SDL
driver, but rendering runs on the main thread, not an SDL thread or SID IRQ.
The selected PCM slot negotiates 22,050 Hz, stereo signed 16-bit and 16 voices.
The original game still owns VOC decoding, voice allocation, priorities,
distance/panning, volume and completion callbacks.

`prepare-effects-source.py` derives an independent source tree from the music
candidate, restores SoundStartup and selects the cooperative PCM driver. The
original notices remain intact. The patch is saved as `build/effects-source.patch`.
The earlier menu/music source trees and successful release artifacts are retained.

`radduke_audio_render` first renders MIDI/SF2, then adds effects with signed-16
saturation. The existing SID ring/LUT path consumes the result. Effects remain
available when the private SoundFont is absent; only MIDI synthesis is silent.
The host run without a font confirmed nonzero real effects PCM. No new codec or
audio library was downloaded. JFAudioLib/VOC/MultiVoc retain their existing
GPL-2.0-or-later grants; project glue is GPL-3.0-or-later. The combined Build/GPL
publication issue remains unresolved, and all game/SF2/PCM assets stay private.

The FX stereo field is Duke's existing positional mix. Wolf's `STEREO_WIDTH` key
remains accepted for configuration compatibility, but does not override Duke's
effect panning. `MIDI_STEREO_WIDTH` only affects music automatic placement.
The SID configuration/selection, timer/blitter, GPIO, DMA/PAL and SD timing
sources were not changed. The same 100 ms output lookahead remains; initial
sample loads can still cause underruns or input-to-audio latency.

## Verification

- PCM bridge unit tests cover format negotiation, division callbacks, chunk
  identity, critical-section exclusion, stop and signed-16 saturation.
- The real pinned decoder/mixer played private PISTOL.VOC and KICKHIT.VOC copies,
  completed voices, invoked callbacks and drained to silence. The test passed
  natively on Windows and Linux; no synthetic waveform substituted for the VOC.
- Real host menu and E1L1 runs produced effects alongside the original-pan music.
  All twelve movement/fire/run/jump/crouch/use/release state checks passed.
- The fontless host run retained audible-in-PCM effects, with no music synthesis.
- The gameplay run recorded 226,048 effect frames and 883 mixer callbacks. Nine
  mixed samples reached the saturating limit; the pure-effects run recorded none.
  This does not establish physical loudness or distortion quality.
- The complete 59-unit ARM64 core/firmware link has no undefined ELF symbols.
  Build logs, source hashes, decoder fixtures' hashes and NOT_TESTED hardware
  metadata are saved in `references/effects-candidate-05/`.

```sh
python3 tools/audit-midi-pan.py
python3 tools/extract-effects.py
python3 tools/prepare-effects-source.py
bash tools/test-effects.sh
RADDUKE_MUSIC=1 RADDUKE_STEREO=1 RADDUKE_EFFECTS=1 python3 tools/build-menu-core.py host
bash tools/run-effects-host.sh
python3 tools/verify-effects-host.py
bash tools/build-effects-firmware.sh
```

The original-pan evidence file used by the verifier is generated with
`tests/test_original_pan.c`, the same music/FIR sources and the private font.
Its successful native output is under `build/effects-tests/original-pan-final.*`.
Host run/staging scripts refuse unknown overwrites and existing backups; staging
has already been performed for this candidate. Do not rerun it blindly.

For the physical test, try menu feedback, several pistol shots, jump/landing and
a door. Check music continuity, left/right outputs, missing/stuck sounds,
distortion and responsiveness. Keep boot.log and duke3d.log, and report the SID
models, kernel hash and duration. The previous music/stereo user reports do not
establish this new effects candidate's stability or full-map behavior.
