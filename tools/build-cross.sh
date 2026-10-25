#!/usr/bin/env bash
# Optional Linux -> Windows x64 build. No driver installation.
set -euo pipefail
cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.."
compiler="${MINGW_CC:-x86_64-w64-mingw32-gcc}"
cxx="${MINGW_CXX:-x86_64-w64-mingw32-g++-posix}"
if ! command -v "$compiler" >/dev/null 2>&1 && [[ -x .toolchain/mingw/usr/bin/x86_64-w64-mingw32-gcc-win32 ]]; then compiler="$PWD/.toolchain/mingw/usr/bin/x86_64-w64-mingw32-gcc-win32"; fi
if ! command -v "$cxx" >/dev/null 2>&1 && [[ -x .toolchain/mingw/usr/bin/x86_64-w64-mingw32-g++-posix ]]; then cxx="$PWD/.toolchain/mingw/usr/bin/x86_64-w64-mingw32-g++-posix"; fi
commit=b66d02d57e32cc8595369c53418b843e958649b4
archive=.deps/vigem/source.zip
mkdir -p build .deps/vigem/compat
if [[ ! -f "$archive" ]]; then curl -fL "https://codeload.github.com/nefarius/ViGEmClient/zip/$commit" -o "$archive"; fi
python3 - <<'PY'
from pathlib import Path
import hashlib,zipfile
archive=Path('.deps/vigem/source.zip')
assert hashlib.sha256(archive.read_bytes()).hexdigest()=='f1d34de29007e0fb17a15f1656c8b84c500bf33aee4e30d8317df1368e76e36c', 'Official source SHA256 mismatch'
source=Path('.deps/vigem/ViGEmClient-b66d02d57e32cc8595369c53418b843e958649b4')
if not source.exists():
    with zipfile.ZipFile(archive) as z:z.extractall('.deps/vigem')
Path('.deps/vigem/compat/Windows.h').write_text('#include <windows.h>\n')
Path('.deps/vigem/compat/SetupAPI.h').write_text('#include <setupapi.h>\n')
PY
"$compiler" -std=c11 -O2 -Wall -Wextra -Wpedantic -Werror -static -static-libgcc src/main.c src/mapping.c src/platform_windows.c -lws2_32 -ldinput8 -ldxguid -lwinmm -lsetupapi -o build/boreal-wheel.exe
"$cxx" -std=c++17 -O2 -shared -static -static-libgcc -static-libstdc++ -DVIGEM_DYNAMIC -DVIGEM_EXPORTS -I.deps/vigem/compat -I".deps/vigem/ViGEmClient-$commit/include" ".deps/vigem/ViGEmClient-$commit/src/ViGEmClient.cpp" -lsetupapi -o build/ViGEmClient.dll
cp ".deps/vigem/ViGEmClient-$commit/LICENSE" build/LICENSE-ViGEmClient.txt
echo 'Windows x64: build/boreal-wheel.exe + build/ViGEmClient.dll'
