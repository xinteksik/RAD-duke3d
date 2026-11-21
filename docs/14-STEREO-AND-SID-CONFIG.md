# Stereo and SID configuration — 0.0.4-stereo01

Later update: the user reported music plays and stereo seems functional; exact
channel separation/duration/hash were not supplied. The current SD contains
[0.0.5-audio01 with effects and original MIDI pan](15-EFFECTS-AND-ORIGINAL-PAN.md).
The following is the retained stereo01 preparation report.

The user confirmed that music plays on RAD with **0.0.3-music01**, then requested
Wolf-compatible stereo and SID settings through CFG. The exact user statement is
preserved in `references/hardware-tests/0.0.3-music01.json`; no independent card
hash, duration or stereo-channel result was supplied.

The new **0.0.4-stereo01** candidate is staged in private `SD/`, with the prior
working music payload saved in `build/sd-backups/0.0.3-music01/`.
New kernel: 1,232,096 bytes, SHA-256
`f04fd84ddf13da84e4241c67b45585db56d35815585a753e67853783e256c8ba`.
**Physical stereo listening is not yet confirmed.** The silent alpha archives
remain unchanged. No physical card was written or ejected.

## AUDIO.CFG

Edit `SD/RADDUKE/AUDIO.CFG` on the staging tree, or `RADDUKE/AUDIO.CFG` on the
card, and reboot. The file is read once when audio starts. It uses the same
strict keys, address checks and models as the frozen Wolf parser. Comments start
with `#`; keys/values are uppercase. Unknown keys, invalid values, embedded NUL,
oversized files or malformed lines discard the whole configuration.

The supplied preset matches the frozen Wolf reference's confirmed mixed-SID setup:

```ini
STEREO=ON
SID1_ADDRESS=$D400
SID1_TYPE=8580
SID2_ADDRESS=$D500
SID2_TYPE=6581
STEREO_WIDTH=100
MIDI_STEREO_WIDTH=70
```

Match the addresses and types to the actual hardware. SID1 is left, SID2 is
right; connect their audio outputs accordingly. Without stereo, SID1 carries
the mono average. Both model fields accept `AUTO`, `6581`, `8580`, `VOLUME` or
`ULTISID`. UltiSID remains experimental; its curve is not for physical SID chips.
The existing C64CAST notice/table are retained. Presets are in `config/` and
`SD/RADDUKE/CONFIG-EXAMPLES/`, including conservative mono/AUTO and mono 8580.

Addresses are hexadecimal, 32-byte-aligned bases in `$D400`–`$D7E0` or
`$DE00`–`$DFE0`. Same stereo addresses and VIC/CIA ranges are rejected. No address
scanning is introduced. SID banks must decode separately: SID2 must not accept
SID1's bank. The retained transport writes SID2 first and SID1 last.

Missing/invalid CFG uses SID1 `$D400`/AUTO mono. If requested SID1 initialization
fails, the inherited routine retains the primary output and stereo stays off.
If SID2 initialization/detection fails, output stays mono. AUTO reuses Wolf's
existing SID tests; it is a type test, not proof of independent address decoding.
An explicit matching primary model preserves its boot-established state; the
existing implementation also retains primary SIDKick handling under AUTO.

`RADDUKE/boot.log` records requested and selected types/addresses, original boot
model, success/fallback and the applied MIDI width. These selection routines and
their IRQ lock ordering are reused, not rewritten. SID timer/blitter, PAL/DMA,
GPIO, boot config and SD timings are byte-identical to the previous sources.

## Stereo placement

`MIDI_STEREO_WIDTH` accepts 0–100. It uses Wolf's automatic melodic-channel
placement (70 by default when stereo is available). Bass/tuba programs and MIDI
percussion stay centered unless explicitly panned. MIDI CC10/42 takes precedence
over automatic placement; controller reset restores the automatic position.
Width 0 preserves MIDI/SoundFont panning without automatic spread; it is not a
command to collapse a genuine stereo SoundFont to mono. Mono hardware output
still averages left/right. `STEREO=OFF` disables the second SID regardless of width.

`STEREO_WIDTH` is retained for CFG compatibility with Wolf's digital-effect
positioning. Duke effects are still disabled, so it does not alter this music-only
mix. Music volume/pause/looping and all gameplay keys are retained.

## Implementation, evidence and limits

- Parser and its original regression tests were copied independently from frozen
  Wolf, with project symbol renames. A bounded file loader rejects partial/invalid
  input before applying SID settings. Provenance: `references/stereo-candidate-04/`.
- Startup tests simulate primary/secondary failures, missing/invalid/oversized/NUL
  CFG and IRQ lock pairing. Ring tests verify configuration occurs with the sample
  player stopped, then preserve the existing prefill and LUT conversion behavior.
- Actual synthesizer tests verify automatic left/right placement, explicit-pan
  precedence/reset, centered bass/percussion and width limits. All seven original
  MIDI tracks again passed full loop/play-once, chunk, pause and volume regressions.
- The actual host engine plays menu/E1L1 music with stereo requested, and defaults
  to mono requests for missing/invalid CFG. Captured PCM has nonzero left/right
  content and channel differences. Host PCM is before hardware mono downmix and
  does not prove physical SID detection or channel separation.
- All twelve gameplay-input checks passed. The 58-unit ARM64 core and firmware
  link without undefined symbols. A separate NOT_TESTED hardware record is saved.
- The compiler cache now includes project, third-party and prepared upstream
  headers, preventing stale objects after shared audio-header changes.

```sh
bash tools/test-stereo.sh
RADDUKE_TEST_OUT=build/stereo-tests/regression bash tools/test-music.sh
RADDUKE_MUSIC=1 RADDUKE_STEREO=1 python3 tools/build-menu-core.py host
bash tools/run-stereo-host.sh
python3 tools/verify-stereo-host.py
bash tools/build-stereo-firmware.sh
```

Host evidence directories and the SD staging helper refuse overwriting existing
results/backups. The latest staging step has already been performed. Raw PCM,
GRP, MIDI and SoundFont remain private. Wolf was read only; no new library or
data distribution permission is asserted, and the combined Build/GPL publication
issue remains open. This is not a new public release.

For the next physical test, check both outputs, menu-to-level music, intended SID
models, mono fallback, distortion/dropouts and responsiveness. Keep boot.log and
duke3d.log. The user's confirmation of music01 does not establish the new stereo
configuration's behavior, performance or long-duration stability.
