"""Build the pinned real mixer test on native Windows or Linux."""
import subprocess
from pathlib import Path
root=Path(__file__).resolve().parents[1]
source=root/'build/effects-source';out=root/'build/effects-tests'
out.mkdir(exist_ok=True)
files=['asssys.c','drivers.c','driver_nosound.c','fx_man.c','multivoc.c','mix.c','mixst.c','pitch.c','vorbis.c']
cmd=['gcc','-std=c99','-O2','-g','-Werror=implicit-function-declaration','-DUSE_ASM=0','-DRADDUKE_PLATFORM','-I'+str(root/'src'),
     '-I'+str(source/'jfaudiolib/include'),'-I'+str(source/'jfaudiolib/src'),
     str(root/'tests/test_multivoc.c'),str(root/'src/radduke_pcm.c'),*[str(source/'jfaudiolib/src'/p) for p in files],
     '-lm','-o',str(out/('multivoc.exe' if __import__('os').name=='nt' else 'multivoc'))]
p=subprocess.run(cmd,text=True,capture_output=True)
(out/'build.log').write_text(p.stdout+p.stderr)
if p.returncode:print(p.stdout+p.stderr)
raise SystemExit(p.returncode)
