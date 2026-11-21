"""Prepare a private, patch-tracked JFDuke3D source copy for RAD services."""
import difflib
import json
from pathlib import Path
import shutil
import sys

root = Path(__file__).resolve().parents[1]
out = root / 'build/menu-source'
if out.exists():
    assert '--refresh' in sys.argv or ('--resume-preparation' in sys.argv and not (root / 'build/menu-source.patch').exists())
else:
    shutil.copytree(root / 'upstream/jfduke3d', out, ignore=shutil.ignore_patterns('.git'))
diffs = []
def patch(name, pairs):
    p = out / name
    before = (root / 'upstream/jfduke3d' / name).read_bytes().decode('latin1').replace('\r\n', '\n')
    after = before
    for old, new in pairs:
        assert after.count(old) == 1, (name, old, after.count(old))
        after = after.replace(old, new)
    p.write_bytes(after.encode('latin1'))
    diffs.extend(difflib.unified_diff(before.splitlines(True), after.splitlines(True),
                                     fromfile='a/' + name, tofile='b/' + name))
patch('src/game.c', [
    ('if(ud.warp_on == 0)\n        Logo();', 'if(ud.warp_on == 0) {\n        ready2send = 0;\n        initspritelists();\n        ps[myconnectindex].cursectnum = -1;\n        ud.camerasprite = -1;\n        setview(0,0,xdim-1,ydim-1);\n        flushperms();\n        clearallviews(0L);\n        setgamepalette(&ps[myconnectindex], palette, 0);\n        backtomenu(); /* RADDUKE: live menu, no intro/demo playback */\n    }'),
    ('if( ud.warp_on == 0 && playback() )', 'if( 0 && ud.warp_on == 0 && playback() )'),
    ('displayrooms(screenpeek,i);\n        displayrest(i);',
     'if (ps[screenpeek].cursectnum < 0) { drawbackground(); menus(); }\n        else { displayrooms(screenpeek,i); displayrest(i); } /* RADDUKE: no unloaded world/HUD */'),
    ('SoundStartup();\n        buildprintf("Checking music inits.\\n");\n        MusicStartup();\n        loadtmb();',
     'FXDevice = MusicDevice = -1; /* RADDUKE: no audio initialization */\n        SoundToggle = MusicToggle = 0;\n        buildputs("RADDUKE sound and music disabled.\\n");'),
])
patch('src/config.c', [('int32 ScreenWidth = 640;', 'int32 ScreenWidth = 320;'),
                       ('int32 ScreenHeight = 480;', 'int32 ScreenHeight = 200;'),
                       ('int32 ForceSetup = 1;', 'int32 ForceSetup = 0;')])
patch('jfbuild/include/compat.h', [
    ('#if defined(__linux) || defined(__HAIKU__)',
     '#if defined(RADDUKE_PLATFORM)\n# define B_LITTLE_ENDIAN 1\n# define B_BIG_ENDIAN 0\n# define B_ENDIAN_C_INLINE 1\n#elif defined(__linux) || defined(__HAIKU__)')])
patch('jfbuild/src/compat.c', [
    ('#include "compat.h"', '#include "compat.h"\n#include "radduke_pathstat.h"'),
    ('DIR *dir;\n#endif\n\tstruct Bdirent', 'DIR *dir;\n    char *radduke_path;\n#endif\n\tstruct Bdirent'),
    ('dirr->dir = opendir(name);', 'dirr->radduke_path = strdup(name);\n    if (!dirr->radduke_path) { free(dirr); return NULL; }\n\tdirr->dir = opendir(name);'),
    ('if (dirr->dir == NULL) {\n\t\tfree(dirr);', 'if (dirr->dir == NULL) {\n        free(dirr->radduke_path);\n\t\tfree(dirr);'),
    ('fstatat(dirfd(dirr->dir), de->d_name, &st, 0)', 'radduke_stat_prefix(dirr->radduke_path, strlen(dirr->radduke_path), de->d_name, &st)'),
    ('closedir(dirr->dir);', 'closedir(dirr->dir);\n    free(dirr->radduke_path);')])
patch('jfbuild/src/kplib.c', [
    ('#include <sys/stat.h>', '#include <sys/stat.h>\n#include "radduke_pathstat.h"'),
    ('fstatat(dirfd(hfind), findata->d_name, &st, 0)',
     'radduke_stat_prefix(wildst, wildstpathleng, findata->d_name, &st)')])
patch('jfaudiolib/src/asssys.c', [
    ('void ASS_Sleep(int msec)', 'extern void radduke_wait_ms(unsigned);\nvoid ASS_Sleep(int msec)'),
    ('select(0, NULL, NULL, NULL, &tv);', 'if (msec > 0) radduke_wait_ms((unsigned)msec);')])
(root / 'build/menu-source.patch').write_text(''.join(diffs))
print('Prepared private menu source with complete patch log.')
