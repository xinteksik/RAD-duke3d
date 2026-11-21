# Game data and license boundaries

Original private game files are in `resources/`; they are excluded from Git and
release assets. The layout cleanup changed directory names only, not game-file
bytes. Inventories are in `resources/DATA-INVENTORY.json`,
`references/DATA-VERIFIED.json` and `references/DATA-SHAREWARE13.json`.

| Local set | GRP bytes | CRC32 | Status |
|---|---:|---|---|
| `resources/DUKE3D/` | 10,442,980 | `912e1e8d` | Shareware **1.1**; CON parsing fails |
| `resources/3dduke13/DN3DSW13.SHR` | 11,035,779 after extraction | `983ad923` | Separate shareware **1.3D**; host menu/E1L1 verified |

The 1.1 GRP has KenSilverman signature, 183 entries, six E1 maps, seven MIDI
entries and thirteen ART entries. Its SHA-256 is
`b218feef4a7b0e48027de17746bd744dc0624b275b9dac0ea8a726f85b48887f`.
Its external GAME/DEFS/USER.CON files match the archive entries. Structural
validity is not proof of compatibility with the selected parser.

The separate SHR file is a ZIP, not an executable that needs to be installed.
It was unpacked only into `build/data-shareware13/`; no DOS installer was run.
Its SHA-256 is `a6c010e735a8f8d910807d41985f16379583cd9d81f14a1e4634f30d04b881e4`.
The 1.3D GRP has 215 entries and SHA-256
`f943d0c2e2a0803a644a2107c81ea897dec87596d9dd1a6a432131ad6f5818d6`.
Run tests on copies. Do not mix external CON files from different editions.

Duke's MIDI/VOC and any SoundFonts have their own rights. The Wolf IMF converter
is not required for Duke's embedded MIDI. Audio is disabled in this alpha.

## Distribution rules

Do not include GRP, CON, ART, MAP, RTS, saves, MIDI/VOC, SoundFonts, DOS EXE or
private captured images in public packages. Shareware is not blanket permission
to extract and repackage individual assets; retain its original terms.

Review grants separately for JFDuke3D, JFBuild, JFMACT/JFAudioLib, RAD, Circle,
circle-stdlib/Newlib and any inherited glue. Do not assign one license to the
whole project. The Build noncommercial/distribution restrictions conflict with
ordinary GPL combined-distribution requirements; no covering exception has been
documented for this firmware. See [the revision-specific review](06-LICENSE-REVIEW.md).
The existence of an upstream release or the Wolf licensing inventory does not
clear the Duke combination. The local SD is private; release SD assets omit data.
