#!/usr/bin/env python3
"""Package already-built Windows binaries and local UI. Installs nothing."""
from pathlib import Path
import hashlib
import shutil
import zipfile
ROOT=Path(__file__).resolve().parents[1]
DEST=ROOT/'dist/boreal-wheel-windows-x64'
DEST.mkdir(parents=True,exist_ok=True)
for name in ['boreal-wheel.exe','ViGEmClient.dll']:
    shutil.copy2(ROOT/'build'/name,DEST/name)
shutil.copytree(ROOT/'web',DEST/'web',dirs_exist_ok=True)
for name in ['README.md','LICENSE','THIRD_PARTY_NOTICES.md']:
    shutil.copy2(ROOT/name,DEST/name)
shutil.copytree(ROOT/'docs',DEST/'docs',dirs_exist_ok=True)
shutil.copytree(ROOT/'examples',DEST/'examples',dirs_exist_ok=True)
shutil.copy2(ROOT/'docs/LICENSE-ViGEmClient.txt',DEST/'LICENSE-ViGEmClient.txt')
licenses=DEST/'LICENSES-MinGW'
licenses.mkdir(exist_ok=True)
for name in ['mingw-w64-common','gcc-mingw-w64-base']:
    candidates=[ROOT/'.toolchain/mingw/usr/share/doc'/name/'copyright',Path('/usr/share/doc')/name/'copyright']
    path=next((item for item in candidates if item.is_file()),None)
    if path is None:raise SystemExit(f'Notificação de runtime ausente: {name}/copyright. Instale os pacotes MinGW antes de empacotar.')
    shutil.copy2(path,licenses/(name+'.txt'))
for name in ['GPL-3','LGPL-3']:
    path=Path('/usr/share/common-licenses')/name
    if not path.is_file():raise SystemExit(f'Licença de runtime ausente: {path}')
    shutil.copy2(path,licenses/(name+'.txt'))
for name in ['run.cmd','run-demo.cmd']:
    options=' --demo' if name=='run-demo.cmd' else ''
    (DEST/name).write_bytes(('@echo off\r\ncd /d "%~dp0"\r\nstart "" "http://127.0.0.1:4580"\r\nboreal-wheel.exe'+options+'\r\npause\r\n').encode('ascii'))
(DEST/'LEIAME.md').write_text('''# Boreal Wheel 0.2.0 — Windows x64

1. Extraia todos os arquivos para uma pasta fixa, por exemplo C:/BorealWheel.
2. Confira o volante físico em joy.cpl e instale o ViGEmBus oficial.
3. Execute run.cmd, selecione Dispositivo e confira a leitura.
4. No HidHide, permita o caminho completo de boreal-wheel.exe e oculte somente o volante físico.
5. Clique Ativar ponte, confira Xbox 360 virtual ativo e abra o jogo como controle/gamepad.

O programa depende do driver ViGEmBus instalado separadamente; ViGEmClient.dll é apenas a biblioteca cliente.
run-demo.cmd explora a interface sem hardware e bloqueia a saída para jogos.
Mantenha o terminal aberto enquanto joga; Ctrl+C encerra o processo.

## Documentação completa, incluída neste pacote

- [Instalação, ViGEmBus e HidHide](docs/INSTALACAO.md)
- [Configuração, mapeamento e calibração](docs/CONFIGURACAO.md)
- [Exportação, importação, perfis antigos e backups](docs/PERFIS.md)
- [Solução de problemas](docs/SOLUCAO-DE-PROBLEMAS.md)
- [Visão geral e limites](README.md)

A migração do Project B exige adicionar o novo boreal-wheel.exe nas exceções do HidHide.
Preserve seu profile.ini ao atualizar. Códigos PB2. e JSON antigos versão 2 continuam importáveis;
novos perfis usam BW2. ou JSON boreal-wheel.
A ponte inicia desligada. O perfil é salvo automaticamente na pasta de execução.

Esta distribuição foi compilada para Windows x64, com testes do núcleo e da interface no Linux.
A versão renomeada ainda requer conferência com volante físico e jogo no Windows.
''',encoding='utf-8')
manifest=[]
for path in sorted(DEST.rglob('*')):
    if path.is_file() and path.name!='SHA256SUMS.txt':manifest.append(hashlib.sha256(path.read_bytes()).hexdigest()+'  '+path.relative_to(DEST).as_posix())
(DEST/'SHA256SUMS.txt').write_text('\n'.join(manifest)+'\n')
archive=ROOT/'dist/boreal-wheel-windows-x64.zip'
with zipfile.ZipFile(archive,'w',zipfile.ZIP_DEFLATED,compresslevel=9) as z:
    for path in sorted(DEST.rglob('*')):
        if path.is_file():z.write(path,path.relative_to(DEST.parent))
print(f'{archive} ({archive.stat().st_size/1024/1024:.2f} MiB)')
