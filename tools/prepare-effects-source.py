"""Use the already-pinned MultiVoc/VOC implementation with a cooperative driver."""
import difflib
from pathlib import Path
import shutil
root=Path(__file__).resolve().parents[1]
base=root/'build/music-source';out=root/'build/effects-source'
assert base.is_dir()
if not out.exists():shutil.copytree(base,out)
diffs=[]
def patch(name,pairs):
    before=(base/name).read_text(encoding='latin1');after=before
    for old,new in pairs:
        assert after.count(old)==1,(name,old,after.count(old))
        after=after.replace(old,new)
    (out/name).write_text(after,encoding='latin1',newline='\n')
    diffs.extend(difflib.unified_diff(before.splitlines(True),after.splitlines(True),fromfile='a/'+name,tofile='b/'+name))
patch('src/game.c',[
    ('FXDevice = -1; SoundToggle = 0; /* RADDUKE: effects remain disabled */',
     'FXDevice = 1; SoundToggle = 1; /* RADDUKE: cooperative PCM slot */\n        NumVoices = 16; NumChannels = 2; NumBits = 16; MixRate = 22050;\n        SoundStartup();'),
    ('RADDUKE MIDI/SF2 music enabled; effects disabled.', 'RADDUKE MIDI/SF2 and MultiVoc effects enabled.')])
patch('jfaudiolib/src/drivers.c',[
    ('#include "drivers.h"','#include "drivers.h"\n#include "radduke_pcm.h"'),
    ('"No Sound",','"RAD DUKE3D cooperative PCM",'),
    ('#ifdef _WIN32\n# include "driver_directsound.h"','#if defined(_WIN32) && !defined(RADDUKE_PLATFORM)\n# include "driver_directsound.h"'),
    ('#if defined _WIN32 && !defined NO_DIRECTSOUND','#if defined _WIN32 && !defined NO_DIRECTSOUND && !defined RADDUKE_PLATFORM'),
    ('#ifdef _WIN32\n        WinMMDrv_GetError,','#if defined(_WIN32) && !defined(RADDUKE_PLATFORM)\n        WinMMDrv_GetError,'),
    *[(name,replace) for name,replace in [
        ('        NoSoundDrv_PCM_Init,','        radduke_pcm_init,'),
        ('        NoSoundDrv_PCM_Shutdown,','        radduke_pcm_shutdown,'),
        ('        NoSoundDrv_PCM_BeginPlayback,','        radduke_pcm_begin,'),
        ('        NoSoundDrv_PCM_StopPlayback,','        radduke_pcm_stop,'),
        ('        NoSoundDrv_PCM_Lock,','        radduke_pcm_lock,'),
        ('        NoSoundDrv_PCM_Unlock,','        radduke_pcm_unlock,')]]])
(root/'build/effects-source.patch').write_text(''.join(diffs))
print('EFFECTS_SOURCE_PREPARED')
