#!/usr/bin/env python3
"""Package already-built Windows binaries and local UI. Installs nothing."""
from pathlib import Path
import hashlib
import os
import shutil
import zipfile
ROOT=Path(__file__).resolve().parents[1]
LANGUAGE=os.environ.get('BW_LANG','en')
def text(english,portuguese):return portuguese if LANGUAGE=='pt-BR' else english
DEST=ROOT/'dist/boreal-wheel-windows-x64'
DEST.mkdir(parents=True,exist_ok=True)
for name in ['boreal-wheel.exe','ViGEmClient.dll']:
    shutil.copy2(ROOT/'build'/name,DEST/name)
shutil.copytree(ROOT/'web',DEST/'web',dirs_exist_ok=True)
for name in ['README.md','README.pt-BR.md','GETTING_STARTED.md','LEIAME.md','LICENSE','THIRD_PARTY_NOTICES.md']:
    shutil.copy2(ROOT/name,DEST/name)
shutil.copytree(ROOT/'docs',DEST/'docs',dirs_exist_ok=True)
shutil.copytree(ROOT/'examples',DEST/'examples',dirs_exist_ok=True)
shutil.copy2(ROOT/'docs/LICENSE-ViGEmClient.txt',DEST/'LICENSE-ViGEmClient.txt')
licenses=DEST/'LICENSES-MinGW'
licenses.mkdir(exist_ok=True)
for name in ['mingw-w64-common','gcc-mingw-w64-base']:
    candidates=[ROOT/'.toolchain/mingw/usr/share/doc'/name/'copyright',Path('/usr/share/doc')/name/'copyright']
    path=next((item for item in candidates if item.is_file()),None)
    if path is None:raise SystemExit(text(f'Missing runtime notice: {name}/copyright. Install MinGW packages before packaging.',f'Notificação de runtime ausente: {name}/copyright. Instale os pacotes MinGW antes de empacotar.'))
    shutil.copy2(path,licenses/(name+'.txt'))
for name in ['GPL-3','LGPL-3']:
    path=Path('/usr/share/common-licenses')/name
    if not path.is_file():raise SystemExit(text(f'Missing runtime license: {path}',f'Licença de runtime ausente: {path}'))
    shutil.copy2(path,licenses/(name+'.txt'))
for name,options in [('run.cmd',''),('run-demo.cmd',' --demo'),('run-pt-BR.cmd',' --lang pt-BR'),('run-demo-pt-BR.cmd',' --demo --lang pt-BR')]:
    (DEST/name).write_bytes(('@echo off\r\ncd /d "%~dp0"\r\nstart "" "http://127.0.0.1:4580"\r\nboreal-wheel.exe'+options+'\r\npause\r\n').encode('ascii'))
manifest=[]
for path in sorted(DEST.rglob('*')):
    if path.is_file() and path.name!='SHA256SUMS.txt':manifest.append(hashlib.sha256(path.read_bytes()).hexdigest()+'  '+path.relative_to(DEST).as_posix())
(DEST/'SHA256SUMS.txt').write_text('\n'.join(manifest)+'\n')
archive=ROOT/'dist/boreal-wheel-windows-x64.zip'
with zipfile.ZipFile(archive,'w',zipfile.ZIP_DEFLATED,compresslevel=9) as z:
    for path in sorted(DEST.rglob('*')):
        if path.is_file():z.write(path,path.relative_to(DEST.parent))
print(f'{archive} ({archive.stat().st_size/1024/1024:.2f} MiB)')
