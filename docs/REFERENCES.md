# Technical references

**English** | [Português (Brasil)](pt-BR/REFERENCES.md)

References for the original implementation. ViGEmBus, HidHide and physical-driver guidance was checked again on 2026-10-25. The localization update keeps the same external dependencies.

- [Thrustmaster — Ferrari 458 Italia Xbox 360](https://support.thrustmaster.com/en/product/ferrari458italia-en/): model identification and Microsoft Xbox 360 driver.
- [Thrustmaster KB 101 — Xbox 360 device not detected](https://support.thrustmaster.com/en/kb/101-en/): standard driver association and Windows diagnostics.
- [Microsoft — XInput and DirectInput](https://learn.microsoft.com/en-us/windows/win32/xinput/xinput-and-directinput): combined triggers in DirectInput and input APIs.
- [Microsoft — XInput versions](https://learn.microsoft.com/en-us/windows/win32/xinput/xinput-versions): operating-system XInput versions.
- [Linux upstream — xpad.c](https://github.com/torvalds/linux/blob/master/drivers/input/joystick/xpad.c): Ferrari 458 Racing Wheel IDs `24C6:5B00` and `24C6:5B03`, and Xbox 360 axis conventions.
- [Nefarius — ViGEmClient](https://github.com/nefarius/ViGEmClient): native ABI, XUSB structure and virtual-driver integration. Pinned commit: `b66d02d57e32cc8595369c53418b843e958649b4`.
- [Nefarius — ViGEmBus official releases](https://github.com/nefarius/ViGEmBus/releases): virtual Xbox 360 device dependency.
- [Nefarius — ViGEm end of life](https://docs.nefarius.at/projects/ViGEm/End-of-Life/): discontinuation and legacy installation guidance.
- [Nefarius — HidHide setup](https://docs.nefarius.at/projects/HidHide/Simple-Setup-Guide/): preventing simultaneous exposure of physical and virtual devices to games.
- [HidHide 1.5.230 official release](https://github.com/nefarius/HidHide/releases/tag/v1.5.230.0): installer and Windows 10/11 Intel/AMD x64 support.
- [ViGEmBus 1.22.0 final release](https://github.com/nefarius/ViGEmBus/releases/tag/v1.22.0): removal of the legacy updater.

No third-party driver-download sites, 458 Spider projects or TX drivers were used as substitutes for the Xbox 360 Italia.
