"""SCUS_972.05 integrated payload exporter. No external Python packages."""
import argparse, hashlib, struct
from pathlib import Path
BASE, STATE, LIMIT = 0x97000, 0x9f000, 0xa0000
GAME_SHA = '00c9cee7f75b921fda1e848d04f64881b5a0b7841300591cac91371bb23d4f8a'
MENU_HOOKS = [
 (0x17d204, 0x0c0c1e60, 0, 'MenuHook97205'),
 (0x17cd18, 0x0c0cd910, 0, 'MenuInputHook97205'),
 (0x17cd7c, 0x0c0ee810, 0x8e6400b8, 'MenuPauseHook97205'),
 (0x17d1b0, 0x0c0ee810, 0xe78098ec, 'MenuPauseHook97205'),
 (0x1e8d3c, 0x0c0ee810, 0x8f849ddc, 'MenuPauseHook97205'),
]
# Opt in via --features after updating existing main.c/game.h/structs.h
# addresses, layouts AND signatures from retail to debug. Menu works separately.
FEATURE_HOOKS = [
 (0x1ea6c0, 0x0c0a6a14, 0x27a60030, 'hk_CheckDIShoot'),
 (0x2a9d5c, 0x0c0aa280, 0x0240202d, 'hk_HandleFireWeapon'),
 (0x2a9698, 0x0c0aa280, 0x0200202d, 'hk_HandleFireWeapon'),
]
def elf(path):
 data=Path(path).read_bytes()
 if data[:7]!=b'\x7fELF\x01\x01\x01': raise ValueError('Expected little endian ELF32')
 h=struct.unpack_from('<16sHHIIIIIHHHHHH',data)
 if h[2]!=8: raise ValueError('Expected MIPS ELF')
 ph=[struct.unpack_from('<8I',data,h[5]+i*h[9]) for i in range(h[10])]
 sh=[struct.unpack_from('<10I',data,h[6]+i*h[11]) for i in range(h[12])]
 return data,ph,sh

def verify_game(path,hooks):
 data,ph,_=elf(path)
 if hashlib.sha256(data).hexdigest()!=GAME_SHA: raise ValueError('Wrong game ELF: expected uploaded SCUS_972.05')
 for site,word,delay,_ in hooks:
  found=False
  for kind,off,va,pa,fs,ms,flags,align in ph:
   if kind!=1: continue
   if va<LIMIT and va+ms>BASE: raise ValueError('Game overlaps payload cave')
   if va<=site and site+8<=va+fs:
    actual=struct.unpack_from('<II',data,off+site-va)
    if actual[0]!=word or (delay is not None and actual[1]!=delay): raise ValueError('Hook mismatch at %08X'%site)
    found=True
  if not found: raise ValueError('Missing game hook')

def payload(path):
 data,ph,sh=elf(path); chunks=[]; symbols={}
 for s in sh:
  name,kind,flags,va,off,size,link,info,align,entry=s
  if kind==2:
   strings=sh[link]; names=data[strings[4]:strings[4]+strings[5]]
   for pos in range(off,off+size,entry):
    n,value,_,_,_,_=struct.unpack_from('<IIIBBH',data,pos)
    end=names.find(b'\0',n)
    symbols[names[n:end].decode()]=value
  if not flags&2 or not size: continue
  if kind==8:
   if not STATE<=va or va+size>LIMIT: raise ValueError('BSS outside runtime area')
   continue
  if kind!=1: raise ValueError('Unexpected allocated section type')
  dest=va
  if flags&1:
   if not STATE<=va or va+size>LIMIT: raise ValueError('Writable data outside runtime area')
   matches=[p for p in ph if p[0]==1 and p[2]<=va and va+size<=p[2]+p[4]]
   if len(matches)!=1: raise ValueError('Missing initialized-data load image')
   dest=matches[0][3]+va-matches[0][2]
  if dest<BASE or dest+size>STATE: raise ValueError('Payload/load image exceeds cave')
  if off+size>len(data): raise ValueError('Truncated section')
  chunks.append((dest,data[off:off+size]))
 chunks.sort()
 if not chunks or chunks[0][0]!=BASE: raise ValueError('Missing payload base')
 for (a,b),(c,d) in zip(chunks,chunks[1:]):
  if a+len(b)>c: raise ValueError('Overlapping load image')
 high=max(a+len(b) for a,b in chunks)
 image=bytearray((high-BASE+15)&~15)
 for address,part in chunks: image[address-BASE:address-BASE+len(part)]=part
 return image,symbols

def audit(path):
 import re
 for line in Path(path).read_text().splitlines():
  m=re.match(r'^\s*[0-9a-fA-F]+:\s+[0-9a-fA-F]{8}\s+[A-Za-z][\w.]*\s*(.*)',line)
  if not m: continue
  operands=m[1].split('#')[0].split('<')[0]
  operands=re.sub(r'(?<![\w$])(?:0x[0-9a-fA-F]+|[0-9][0-9a-fA-F]*)(?![\w])','',operands)
  if re.search(r'(?<![\w])\$?(?:s[0-8]|fp|gp|f(?:2[0-9]|3[01])|1[6-9]|2[0-3]|28|30)(?![\w])',operands):
   raise ValueError('Reserved register used: '+line)

def main():
 ap=argparse.ArgumentParser();ap.add_argument('--game');ap.add_argument('payload',nargs='?');ap.add_argument('output',nargs='?');ap.add_argument('--features',action='store_true');ap.add_argument('--audit')
 args=ap.parse_args();hooks=MENU_HOOKS+(FEATURE_HOOKS if args.features else [])
 if args.game: verify_game(args.game,hooks)
 if not args.payload:
  if not args.game: ap.error('Provide a payload and output, or --game for verification only')
  print('Debug ELF and hook sites verified.');return
 if not args.output: ap.error('Provide the output PNACH path')
 if args.audit: audit(args.audit)
 image,symbols=payload(args.payload)
 lines=['gametitle=SOCOM SCUS_972.05','[NativeHooks Menu]','// Runtime state is initialized by C, never continuously patched.']
 for offset in range(0,len(image),4):
  lines.append('patch=1,EE,%08X,word,%08X'%(BASE+offset,struct.unpack_from('<I',image,offset)[0]))
 for site,word,delay,name in hooks:
  target=symbols[name]
  if not BASE<=target<BASE+len(image) or target&3: raise ValueError('Bad hook symbol '+name)
  lines.append('patch=1,EE,%08X,word,%08X'%(site,0x0c000000|(target>>2)))
 Path(args.output).write_text('\n'.join(lines)+'\n')
 Path(args.payload).with_suffix('.bin').write_bytes(image)
 print('Generated '+args.output+'; existing feature hooks '+('enabled' if args.features else 'not installed (offset port pending)'))
if __name__=='__main__':
 try:main()
 except (ValueError,KeyError,OSError,struct.error) as error:raise SystemExit(str(error))
