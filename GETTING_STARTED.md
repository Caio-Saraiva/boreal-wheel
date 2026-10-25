# Boreal Wheel 0.3.0 — Windows x64 quick start

**English** | [Português (Brasil)](LEIAME.md)

1. Extract the complete package to a permanent folder such as `C:/BorealWheel`.
2. Check the physical wheel in `joy.cpl` and install the official ViGEmBus driver.
3. Run `run.cmd`, select **Device** and check input.
4. In HidHide, allow the full path to `boreal-wheel.exe` and hide only the physical wheel.
5. Click **Enable bridge**, confirm **Virtual Xbox 360 active** and open the game as a controller/gamepad.

English is the default. Use **Language → Português (Brasil)** in the top bar for the Portuguese interface. `run-pt-BR.cmd` also starts native terminal messages in Portuguese; `run-demo-pt-BR.cmd` is its Demo counterpart.

The driver is installed separately; the included `ViGEmClient.dll` is only the client library. `run-demo.cmd` lets you explore without hardware and blocks game output. Keep the terminal open while playing; `Ctrl+C` stops the process.

## Complete documentation included

- [Installation, ViGEmBus and HidHide](docs/INSTALLATION.md)
- [Configuration, mapping and calibration](docs/CONFIGURATION.md)
- [Export, import, legacy profiles and backups](docs/PROFILES.md)
- [Troubleshooting](docs/TROUBLESHOOTING.md)
- [Overview and limitations](README.md)
- [All Portuguese guides](README.pt-BR.md)

Migrating from Project B requires allowing the new executable in HidHide. Preserve `profile.ini` when updating. Legacy PB2. codes and version 2 JSON remain importable; new profiles use BW2./boreal-wheel. Language changes do not alter the profile format.

The bridge starts disabled. Profiles save automatically in the working folder. This distribution has Linux core/interface tests and a Windows x64 build; verification with a physical wheel and game on Windows is still needed.
