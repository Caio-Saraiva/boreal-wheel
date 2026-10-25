# Development, builds and tests

**English** | [Português (Brasil)](pt-BR/DEVELOPMENT.md)

[README](../README.md) · [API](API.md) · [Licenses](../THIRD_PARTY_NOTICES.md)

## Get the source

```bash
git clone https://github.com/Caio-Saraiva/boreal-wheel.git
cd boreal-wheel
```

The core is C11, the interface is HTML/CSS/JavaScript and the external ViGEmClient library is C++. There is no npm dependency or web framework. `build`, `dist`, `.deps`, `.toolchain` and personal profiles are excluded from Git.

## Linux

On Ubuntu/Debian:

```bash
sudo apt install build-essential python3 cmake
./run.sh --demo
```

The launcher builds `build/boreal-wheel`, uses the repository as its working directory and opens `http://127.0.0.1:4580`. Press `Ctrl+C` to stop.

```bash
./run.sh                       # physical evdev/xpad input
./run.sh --demo                 # simulation with game output blocked
./run.sh --port 4581            # alternative port
./run.sh --demo --lang pt-BR     # Portuguese process messages
BW_NO_BROWSER=1 ./run.sh --demo # no automatic browser launch
./run.sh --build-only           # compile only
CC=gcc ./run.sh --build-only    # choose compiler
```

`.toolchain` is an optional local fallback, not included in a clone and not required. Install a system compiler for a fresh clone.

CMake alternative:

```bash
cmake -S . -B build-cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build-cmake
ctest --test-dir build-cmake --output-on-failure
cd build-cmake
./boreal-wheel --demo
```

CMake copies `web` beside the executable. Linux input requires access to the wheel's event node; see [permissions](TROUBLESHOOTING.md#linux-read-permissions). Virtual output remains Windows-only.

## Windows x64 with Visual Studio

Install **Visual Studio 2022 / Build Tools** with **Desktop development with C++**, Windows SDK and CMake available in the terminal. From PowerShell in the repository:

```powershell
.\build-windows.ps1
.\run-windows.ps1
.\run-windows.ps1 -Demo
.\run-windows.ps1 -Port 4581
# Optional Portuguese process/build messages:
.\build-windows.ps1 -Language pt-BR
.\run-windows.ps1 -Language pt-BR
```

The build configures CMake x64, compiles Release, runs the C mapping test with CTest, downloads the pinned official ViGEmClient ZIP, verifies its SHA256 and builds the DLL. The DLL and its license are copied to `build-windows/Release`.

The first dependency build needs internet. The script does not install ViGEmBus/HidHide. To build only the core:

```powershell
.\build-windows.ps1 -SkipVigem
# Build the DLL separately later:
.\tools\build-vigem-client.ps1
```

Without the DLL, the monitor and Demo can work but the bridge cannot communicate with ViGEmBus. Having the DLL without the installed ViGEmBus driver is also insufficient.

## Cross-compile Linux → Windows

```bash
sudo apt install gcc-mingw-w64-x86-64 g++-mingw-w64-x86-64 curl python3
./tools/build-cross.sh
python3 tools/package-windows.py
```

The script uses `x86_64-w64-mingw32-gcc` and `x86_64-w64-mingw32-g++-posix`. If your distribution provides suffixed names, set `MINGW_CC`/`MINGW_CXX`. A local `.toolchain/mingw` can also serve as a fallback.

Outputs: `build/boreal-wheel.exe` and `build/ViGEmClient.dll`, both x64. The dependency ZIP is cached in `.deps/vigem`, with pinned commit/hash; an unexpected hash aborts the build. Keep that verification.

The Python packager uses these already-built MinGW binaries and copies interface, both sets of guides, examples, licenses and launchers. It creates `GETTING_STARTED.md` in English, `LEIAME.md` in PT-BR and `SHA256SUMS.txt`. Result: `dist/boreal-wheel-windows-x64.zip`. It is not an installer or an MSVC build script.

MinGW runtime notices come from the local toolchain or system packages. Packaging fails if they are missing. Do not include personal INI files or drivers. Publish the ZIP checksum too. Use `BW_LANG=pt-BR` for localized shell/packaging messages; generated packages always include English first and PT-BR second.

## Tests

```bash
./test.sh
# Interface: Firefox and geckodriver on PATH, no extra Python packages
python3 tests/test_ui.py
```

`test.sh` builds the application and mapping tests, then runs native API integration. Tests cover curves, continuity/renormalization, independent pedals, source swapping, buttons, digital pairs, thresholds, persistence, disconnects, profile validation, HTTP restrictions and native language selection.

UI tests use headless Firefox and the real C process in **both English and PT-BR**. They cover numeric/drag calibration, identification, rest measurement, pedal swap, themes, profile import/export, legacy profiles and atomic rejection. They also check English default despite Portuguese browser preferences, live language changes, persistence, accessible labels and untouched user input/settings. Screenshots are updated under `docs/` for English and `docs/pt-BR/images/` for Portuguese. Layouts are checked at 1440 and 390 pixels.

| Variable | Purpose |
| --- | --- |
| `BW_BINARY` | Another native executable for `tests/test_api.py` |
| `GECKODRIVER` | geckodriver executable for UI tests |
| `FIREFOX_BINARY` | Firefox executable for UI tests |

Tests isolate profiles in temporary folders. Linux tests and compilation do not prove driver installation or physical Windows behavior; check the package on Windows with real hardware before treating that integration as validated.

## Source layout and localization

```text
src/bridge.h            Native contracts and BW structures
src/mapping.c           Sources, curves, pedals, buttons and validation
src/main.c              Loop, snapshots, profile and local HTTP server
src/i18n.c              English/PT-BR native messages
src/platform_windows.c  XInput / DirectInput / SetupAPI / ViGEm
src/platform_linux.c    sysfs / evdev / xpad
web/app.js              Interface behavior
web/i18n.js             Language selection and display-text translation
web/locales.js          English source messages and PT-BR translations
examples/               Exportable default profile
tests/                 C, API and Firefox tests
tools/                 Dependency build, cross-build and packaging
docs/                  English guides, screenshots and reference license
docs/pt-BR/             Complete Portuguese guides and screenshots
```

English is the source language. Wrap dynamic interface messages in `t(...)`, add their PT-BR translations in `web/locales.js`, and translate static text/accessibility attributes through `translateTree`. Canvas labels also use `t`. Templates handle formatted backend messages. Only application-owned display text is translated; never translate selectors, API/profile keys, user-entered codes or third-party device names.

Native display messages use `bw_text(...)`; add corresponding English/PT-BR entries in `src/i18n.c`. `--lang en|pt-BR` selects process language before worker startup. Browser language is a separate local-storage preference, not a configuration field.

For a profile format change, update native/browser validation, docs, examples and round-trip tests. Preserve legacy imports where possible. Do not rename or translate third-party source/license text.

## Licenses

Application code is MIT. The DLL comes from [official ViGEmClient](https://github.com/nefarius/ViGEmClient), commit `b66d02d57e32cc8595369c53418b843e958649b4`, with source-ZIP SHA256 `f1d34de29007e0fb17a15f1656c8b84c500bf33aee4e30d8317df1368e76e36c`. See [third-party notices](../THIRD_PARTY_NOTICES.md). ViGEmBus and HidHide are not redistributed.
