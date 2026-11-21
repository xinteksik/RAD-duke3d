"""Derive a separate music candidate from the prepared silent source tree."""
import difflib
from pathlib import Path
import shutil
root = Path(__file__).resolve().parents[1]
base = root / 'build/menu-source'
out = root / 'build/music-source'
assert base.is_dir()
if not out.exists(): shutil.copytree(base, out)
diffs = []
def patch(name, pairs):
    before = (base / name).read_text(encoding='latin1')
    after = before
    for old, new in pairs:
        assert after.count(old) == 1, (name, old, after.count(old))
        after = after.replace(old, new)
    (out / name).write_text(after, encoding='latin1', newline='\n')
    diffs.extend(difflib.unified_diff(before.splitlines(True), after.splitlines(True), fromfile='a/' + name, tofile='b/' + name))
patch('src/game.c', [
    ('FXDevice = MusicDevice = -1; /* RADDUKE: no audio initialization */\n        SoundToggle = MusicToggle = 0;\n        buildputs("RADDUKE sound and music disabled.\\n");',
     'FXDevice = -1; SoundToggle = 0; /* RADDUKE: effects remain disabled */\n        MusicDevice = 0; MusicToggle = 1;\n        MusicStartup();\n        buildputs("RADDUKE MIDI/SF2 music enabled; effects disabled.\\n");'),
    ('    cmenu(0);\n    KB_FlushKeyboardQueue();\n}',
     '    cmenu(0);\n    KB_FlushKeyboardQueue();\n    playmusic(&env_music_fn[0][0]); /* RADDUKE: live menu theme without Logo() */\n}')
])
patch('src/sounds.c', [
    ('#define MUSIC_ID  -65536', '#include "radduke_music.h"\n#define MUSIC_ID  -65536'),
    ('    stopmusic();\n    \n    ext = strrchr(fn,',
     '    stopmusic();\n    radduke_music_name(fn); /* RADDUKE: selected GRP MIDI telemetry */\n    ext = strrchr(fn,'),
    ('    if (ext && !strcasecmp(ext, ".mid")) {', '    if (0 && ext && !strcasecmp(ext, ".mid")) { /* RADDUKE: MIDI-only candidate */'),
    ('    MusicLen = kfilelength( fp );\n    MusicPtr = (char *) malloc(MusicLen);\n    kread( fp, MusicPtr, MusicLen);\n    kclose( fp );',
     '    MusicLen = kfilelength(fp);\n    if (MusicLen < 14 || MusicLen > 1024*1024) { kclose(fp); MusicLen = 0; return; }\n    MusicPtr = (char *)malloc(MusicLen);\n    if (!MusicPtr || kread(fp, MusicPtr, MusicLen) != (int)MusicLen) {\n        kclose(fp); free(MusicPtr); MusicPtr = NULL; MusicLen = 0; return;\n    }\n    kclose(fp);'),
    ('       MusicVoice = FX_PlayLoopedAuto(MusicPtr, MusicLen, 0, 0, 0,\n                                      MusicVolume, MusicVolume, MusicVolume,\n\t\t\t\t      FX_MUSIC_PRIORITY, MUSIC_ID);\n       MusicIsWaveform = 1;',
     '       free(MusicPtr); MusicPtr = NULL; MusicLen = 0; /* RADDUKE: no waveform backend */\n       MusicVoice = -1; MusicIsWaveform = 0;')
])
(root / 'build/music-source.patch').write_text(''.join(diffs))
print('MUSIC_SOURCE_PREPARED', out)
