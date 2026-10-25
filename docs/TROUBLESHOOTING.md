# Troubleshooting

**English** | [Português (Brasil)](pt-BR/TROUBLESHOOTING.md)

[README](../README.md) · [Installation](INSTALLATION.md) · [Profiles](PROFILES.md)

## Which stage failed?

Check in order: **physical driver → input in Boreal Wheel → virtual output → HidHide → game**. Start with the game closed. Temporarily disable HidHide hiding to test the physical wheel in Windows, then re-enable it afterward.

| Symptom | Check first |
| --- | --- |
| Nothing in Device Manager | Cable, USB port, pedal connection and another PC; manufacturer guidance |
| USB detected, but no input | Physical driver, XInput slot, HidHide allowlist |
| Interface responds, but game does not | Device versus Demo, active bridge, ViGEmBus and in-game controller selection |
| Menus skip or actions happen twice | Physical and virtual controllers both visible; HidHide and other remappers |
| Wrong pedal or input at rest | RT/LT sources, inversion, combined DI axis and deadzone |
| Settings disappear after restarting | INI location, write error or multiple instances |

## Page will not open / core offline

- Run `run.cmd` or a development launcher and keep the terminal open. Opening HTML alone does not start the core.
- Use `http://127.0.0.1:4580`, with **http**, without substituting `localhost`. The server validates this address.
- If the browser started too early, reload. After an update, use `Ctrl+F5`.
- If the port is occupied, stop the previous instance. Alternatively, from a terminal in the extracted folder:

```powershell
.\boreal-wheel.exe --port 4581
```

Open `http://127.0.0.1:4581`. For a Windows source build, use `.\run-windows.ps1 -Port 4581`; on Linux, `./run.sh --port 4581`.

**UI assets missing** means `web` was not found in the working directory. Use the supplied launcher or start in the folder containing `web`. **Invalid port** means you need an integer from 1024 to 65535.

## Wheel invisible or USB without input

1. Disable the bridge and HidHide hiding; reconnect the wheel.
2. Check `devmgmt.msc` and `joy.cpl`. Fix the Xbox 360 driver using the [manufacturer's guide](https://support.thrustmaster.com/en/kb/101-en/) first.
3. Under **Diagnostics → Physical XInput source**, try the correct slot from 0 to 3. Automatic mode requires wheel subtype; some drivers present the device as a gamepad.
4. Try **Force DirectInput** only as a fallback. Recognized IDs/models remain limited to the project's target hardware.
5. If re-enabling HidHide breaks input, check the full allowed path to `boreal-wheel.exe` and restart it. Allowing `run.cmd` or the browser is insufficient.

Reconnecting USB, changing other controllers and creating/removing virtual devices can change enumeration. Recheck the physical slot. The application excludes its own virtual slot to avoid feedback.

## Virtual output does not activate

| Message | Action |
| --- | --- |
| Demonstration · game output blocked | Select Device. Demo cannot feed games. |
| Waiting for physical input · output neutralized | Restore physical input; the bridge does not feed games without it. |
| ViGEmClient.dll missing next to the executable | Extract the whole package. Check that the DLL is beside the EXE and can be loaded. |
| Incompatible ViGEmClient.dll · check the x64 build | Use the package's x64 DLL; do not mix architectures/builds. |
| ViGEmBus unavailable · install the official driver | Install/repair the official driver, complete restarts and reopen the application. |
| Could not create a virtual Xbox 360 controller in ViGEmBus | Check the driver and other remappers. Remove unnecessary virtual controllers and restart the process. |
| Could not safely identify the virtual slot | Check the four available XInput slots and conflicts; output is removed when its slot cannot be confirmed. |
| Could not send report to ViGEmBus | Check the driver, disable/enable the bridge and restart the process if needed. |
| Linux preview · XInput output is only available on Windows | Expected on Linux; use the Windows version for this output. |

ViGEmBus connection failures are retried. Creating the virtual device can take a moment; wait for **Virtual Xbox 360 active** before opening the game.

## Double input or game prefers the original wheel

HidHide should hide **only the physical device**, allow Boreal Wheel to read it, and let the game see the virtual controller. Check Applications, the new executable path, Devices and hiding settings using [installation](INSTALLATION.md#configure-hidhide).

Close other remappers and restart the game after creating the virtual controller. If Steam Input or another translator is involved, try the appropriate per-game option to avoid concurrent translation. Games may remember a previous device; select the Xbox 360 controller/gamepad again.

If you accidentally hid the virtual controller, disable the bridge and identify the physical entry by reconnecting the wheel. To undo hiding, clear **Enable device hiding**, reconnect and reopen applications. The [official guide](https://docs.nefarius.at/projects/HidHide/Simple-Setup-Guide/) notes that some XInput clients may still show marked devices.

## Incorrect pedals or steering

- **Swapped pedals:** use **Swap throttle and brake**, then check each output's calibration.
- **Pedal reads 100% when released:** check source and **Invert direction**. An oversized deadzone is not a correction for inverted input.
- **Throttle and brake cancel:** check for DirectInput and a combined-pedals notice. That path cannot recover simultaneous pedal input; prefer XInput, as explained by [Microsoft](https://learn.microsoft.com/en-us/windows/win32/xinput/xinput-and-directinput).
- **Steering saturates too soon:** increase **Reach 100% output at** and check in-game sensitivity.
- **Too much steering movement needed:** lower that limit gradually.
- **Movement at rest:** capture center with the wheel released and cover noise with a suitable deadzone.
- **Identify selects another control:** stabilize inputs, move only one or select the source manually.
- **Capture maximum rejected:** hold the control beyond 5% and at least 2 percentage points above its deadzone.

There is no force feedback or native wheel output in this version. Games can apply their own gamepad filters after Boreal Wheel's adjustments.

## Profile import or persistence fails

Supported codes: `BW2.` and legacy `PB2.`. Supported file: complete version 2 profile JSON. Do not import diagnostic JSON as a profile. Check the message by the field and read [profiles](PROFILES.md).

If changes last only until process shutdown, check write permission and the working directory. Do not run two instances saving the same INI. Back up the file before editing/moving it.

## Language or translated messages

English is the initial default. Select **Português (Brasil)** for PT-BR; this browser preference is separate from the profile. If the choice resets, check browser storage or whether you changed port/browser/profile.

The native process uses English unless started with `--lang pt-BR` (or `BW_LANG=pt-BR`). Both languages retain the same API keys and profile format. Third-party device names, technical identifiers and operating-system error text can retain their original language.

## Linux read permissions

The backend searches known IDs under `/sys/class/input` and opens `/dev/input/eventN`. If USB is detected but access is denied, identify the wheel's event node and test a temporary device-specific permission:

```bash
sudo setfacl -m "u:$(id -un):r" /dev/input/eventN
```

Replace `eventN` with the actual node, which can change on reconnection. This is not a persistent rule. Check your distribution's permissions, udev and `xpad` for a permanent solution. You do not need to run the whole interface as root. Linux still has no XInput output in this project.

## Request help with evidence

Use **Export diagnostics**, inspect the JSON before sharing, and provide the application version, Windows/Linux version, exact wheel model, backend, failed stage, terminal message and hiding state. Say whether `joy.cpl` input works with HidHide disabled. Share the profile separately for mapping/calibration problems, and mention the interface/process language when reporting a translation issue.
