"""Extract private VOC fixtures for real MultiVoc decoder tests."""
import hashlib,json,struct
from pathlib import Path
root=Path(__file__).resolve().parents[1]
data=(root/'SD/RADDUKE/duke3d.grp').read_bytes()
assert hashlib.sha256(data).hexdigest()=='f943d0c2e2a0803a644a2107c81ea897dec87596d9dd1a6a432131ad6f5818d6'
count,=struct.unpack_from('<I',data,12);offset=16+16*count
out=root/'build/effects-assets';out.mkdir(exist_ok=True)
records=[]
for i in range(count):
    raw,n=struct.unpack_from('<12sI',data,16+16*i);name=raw.split(b'\0')[0].decode('ascii')
    assert offset+n<=len(data)
    if name.upper() in ('PISTOL.VOC','SHOTGN.VOC','KICKHIT.VOC'):
        sample=data[offset:offset+n];assert sample.startswith(b'Creative Voice File')
        p=out/name
        if p.exists():assert p.read_bytes()==sample
        else:p.write_bytes(sample)
        records.append({'file':name,'bytes':n,'sha256':hashlib.sha256(sample).hexdigest()})
    offset+=n
assert offset==len(data) and any(r['file']=='PISTOL.VOC' for r in records)
target=root/'references/effects-candidate-05';target.mkdir(exist_ok=True)
(target/'PRIVATE-VOC-INVENTORY.json').write_text(json.dumps({'private_assets_only':True,'files':records},indent=2)+'\n')
print('PRIVATE_VOC_FIXTURES_EXTRACTED',records)
