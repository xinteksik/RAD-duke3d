# Licensing and third-party notices

RAD DUKE3D contains components under different licenses. **No single license is
assigned to the entire project or its combined firmware.** This file is a guide
to the retained notices; it does not replace them, relicense third-party code,
or grant rights to files whose license coverage remains unresolved.

The license and copyright notices in each source file and its originating
component continue to apply. Full retained texts and evidence are in
[references/license-evidence](references/license-evidence/). Prepared release
trees also contain `LICENSES/` and `vendor/` notices; complete source archives
retain the upstream source notices.

## Component licenses

| Component | Retained terms / scope |
|---|---|
| JFDuke3D game | GPL-2.0-or-later according to the inspected game source notices. |
| JFBuild / Build renderer | Ken Silverman's Build Source Code License; Fowler's changes retain the corresponding terms. This is a separate license with noncommercial and distribution restrictions. |
| KPLIB | Its own source-header attribution and use terms; these do not relicense the rest of Build. |
| JFAudioLib / MultiVoc | GPL-2.0-or-later according to the inspected mixer, decoder and driver notices. |
| JFMACT | Some files explicitly state GPL-2.0-or-later; coverage of other files remains unresolved. Consult the revision-specific review. |
| RAD / RAD-Doom, Circle and inherited RAD-Wolf integration | GPL-3.0-or-later where stated in the retained file notices; other component-specific notices remain applicable. |
| circle-stdlib | Retains its GPLv3 license; this is not a uniform license for all bundled runtime objects. |
| Newlib, GCC runtime and boot/support files | Their respective per-file licenses, notices and runtime exceptions. |
| TinySoundFont | MIT; full notice retained in [third_party/tsf.h](third_party/tsf.h). |
| TinyMidiLoader | zlib; full notice retained in [third_party/tml.h](third_party/tml.h). |
| Project adapters and audio glue | Follow each file's existing notices, including inherited GPL-3.0-or-later grants. This index does not assign a blanket grant to unmarked files. |

Exact dependency revisions, inspected grants and open questions are recorded in
[the license review](docs/06-LICENSE-REVIEW.md). Preserve original copyright
notices and license texts when copying or modifying individual components.

"Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman

Ken Silverman's official web site: "http://www.advsys.net/ken"

The retained Build license text is included in the license evidence and as
`LICENSES/BUILDLIC.TXT` in prepared releases.

## Combined firmware publication status

Public distribution of the combined Build/GPL firmware remains **unresolved**.
The current project review has not documented a covering permission or exception
for that combination; some JFMACT grant coverage also requires clarification.
Preparing source and binary archives does not establish redistribution rights.
Keep the combined release artifacts private until those issues are resolved.

This status does not replace, withdraw or narrow rights already granted by the
individual component licenses. No additional permission from upstream authors
is represented by this file.

## Game data and other private assets

Duke Nukem 3D game data, original DOS programs, GRP/CON/ART/MAP/RTS files, saves,
MIDI/VOC, SoundFonts and private captures are not licensed by this repository.
They remain subject to their own rights and terms and are excluded from the
prepared public-facing packages. Shareware status is not blanket permission to
extract and redistribute assets. Supply your own lawfully obtained data as
described in [the data boundaries](docs/04-DATA-AND-LICENSES.md).
