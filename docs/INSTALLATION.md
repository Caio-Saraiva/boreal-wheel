# Windows installation: wheel → Boreal Wheel → game

**English** | [Português (Brasil)](pt-BR/INSTALLATION.md)

[README](../README.md) · [Configuration](CONFIGURATION.md) · [Troubleshooting](TROUBLESHOOTING.md)

## What each component does

```text
Ferrari 458 Italia Xbox 360
  → Windows Xbox 360 driver
  → Boreal Wheel (reads, remaps and calibrates)
  → ViGEmClient.dll → ViGEmBus
  → virtual Xbox 360 controller → game

HidHide: hides the physical wheel from the game while allowing Boreal Wheel to read it.
```

**ViGEmBus** creates the virtual output device. **ViGEmClient.dll** lets the application communicate with that driver. **HidHide** prevents the game from receiving both the original and virtual devices. Installing HidHide alone does not create a virtual controller.

## Requirements

- Windows x64 PC; Windows 11 is the main application target. HidHide 1.5.230 lists support for 64-bit Windows 10/11 on Intel/AMD.
- **Thrustmaster Ferrari 458 Italia for Xbox 360**, connected over USB and readable through the Windows driver.
- Current browser with JavaScript and canvas. Do not open `web/index.html` directly: it requires the native process.
- Administrator permission to install drivers, with restarts when requested. Everyday use of the application does not install drivers.
- An available XInput slot. Other remapping applications can occupy slots too.

## 1. Check the physical wheel

1. Close the game and connect the wheel and pedals. Start with HidHide device hiding disabled.
2. Press `Win+R`, run `devmgmt.msc` and check the device. Resolve a missing device or driver warning before using the bridge.
3. Run `joy.cpl`, open the physical device's **Properties** and test steering, pedals and buttons.
4. If input does not work, follow the [official Thrustmaster Xbox 360 driver article](https://support.thrustmaster.com/en/kb/101-en/). Do not substitute TX/458 Spider drivers for this model.

USB detection does not mean readable input. The adapter depends on the physical driver and does not provide a kernel driver to replace the Windows Xbox 360 driver.

## 2. Install ViGEmBus

1. Open the [official ViGEmBus Setup 1.22.0 release](https://github.com/nefarius/ViGEmBus/releases/tag/v1.22.0).
2. Under **Assets**, download the maintainer's installer, run it and complete installation. Restart if requested.
3. [ViGEm is discontinued](https://docs.nefarius.at/projects/ViGEm/End-of-Life/). Release 1.22.0 removed the legacy updater; follow the maintainer's guidance for old installations showing update alerts.
4. An installer in the application folder does not act as the client DLL: they are different components.

**ViGEmBus unavailable** means the driver is not accessible yet. Check installation and restarts before changing mapper settings.

## 3. Extract and launch Boreal Wheel

Download the ZIP from [Releases](https://github.com/Caio-Saraiva/boreal-wheel/releases/latest). Extract it to a permanent, writable folder such as `C:\BorealWheel`. Do not run it inside the ZIP.

```text
C:\BorealWheel\
  boreal-wheel.exe
  ViGEmClient.dll
  run.cmd, run-demo.cmd
  run-pt-BR.cmd, run-demo-pt-BR.cmd
  web\, docs\, examples\
  README.md, README.pt-BR.md, GETTING_STARTED.md, LEIAME.md
  LICENSE, third-party licenses and SHA256SUMS.txt
```

1. Run `run.cmd` and keep its terminal open.
2. Open `http://127.0.0.1:4580`. If the browser opens before the core is ready, reload.
3. Select **Device**. **Demo** is a simulation and cannot feed games.
4. Confirm **Input detected** and move the controls.
5. If the driver exposes the wheel as a gamepad, try the correct **Diagnostics → Physical XInput source** slot from 0 to 3. Keep the bridge disabled while identifying the original device. Automatic mode looks for the wheel subtype, then tries DirectInput for known models.
6. Prefer XInput for independent pedals. **Force DirectInput** is a diagnostic fallback with combined-pedal limitations.

### Choose a language

The interface starts in English even if the browser prefers Portuguese. Select **Português (Brasil)** in the top-bar **Language** control for PT-BR. The choice is stored in the browser; switching language leaves calibration, controls, typed profile codes and the bridge unchanged.

The native terminal and API messages default to English. `run-pt-BR.cmd` starts those messages in Portuguese. You can also use `boreal-wheel.exe --lang pt-BR`, `./run.sh --lang pt-BR` or `.\run-windows.ps1 -Language pt-BR`. The UI selector is independent and can translate the supported process messages either way.

## Configure HidHide

These steps adapt the [official HidHide guide](https://docs.nefarius.at/projects/HidHide/Simple-Setup-Guide/) to Boreal Wheel. The [official 1.5.230 release](https://github.com/nefarius/HidHide/releases/tag/v1.5.230.0) lists supported systems.

1. Install HidHide from the official release Assets and restart.
2. Open **HidHide Configuration Client**. Under **Applications**, add the full path to `C:\BorealWheel\boreal-wheel.exe`, using your actual installation path.
3. Under **Devices**, with the bridge disabled, disconnect/reconnect the wheel to identify the physical entry. Select only that device.
4. Enable **Enable device hiding** and leave **inverse application cloak** off.
5. Reconnect the wheel, then fully restart Boreal Wheel and the game.

Allow the **native executable**: neither the browser nor `run.cmd` reads the wheel. Keep the virtual controller visible and games outside the allowlist. If Boreal Wheel also loses input, check the allowed executable path.

With only this wheel connected, the physical entry should disappear from `joy.cpl` while the bridge is off; enabling the bridge should show the virtual controller. Boreal Wheel should continue responding to the physical wheel. The official guide notes a visibility limitation for some XInput clients; see [troubleshooting](TROUBLESHOOTING.md) if double input persists.

## 5. Enable the bridge and configure the game

1. Before opening the game, check steering and pedals in the interface.
2. Click **Enable bridge**. Wait for **Virtual Xbox 360 active · slot N** in Diagnostics.
3. Check the virtual controller in `joy.cpl`. It will usually have an Xbox 360 controller name, rather than Boreal Wheel.
4. Open the game and select **Xbox 360 controller / gamepad**. Defaults are LX for steering, RT for throttle and LT for brake.
5. Test while stationary. Avoid two remappers generating output for the same wheel; adjust per-game Steam Input settings if needed to avoid concurrent translation.

The monitor shows calculated output; **virtual active** confirms delivery to ViGEmBus. Movement in Demo does not prove in-game operation.

## Daily use and shutdown

Connect the wheel → `run.cmd` → **Device** → check input → **Enable bridge** → open the game. The profile loads automatically; the bridge starts disabled each time.

To stop, close the game, disable the bridge and press `Ctrl+C` in the terminal. Closing only the browser leaves the process active. A device disconnect neutralizes output and removes the virtual device. Reconnection retries input and, if bridge output is still requested, recreates the output.

## Updates, moving folders and removal

Before updating, [export a profile](PROFILES.md) and copy `profile.ini`. Stop the process and replace the package files while preserving that INI. Reopen and use `Ctrl+F5` in the browser.

**Migrating from Project B:** add `boreal-wheel.exe` to HidHide Applications even if the folder is unchanged: the executable name changed. Copy the old `profile.ini` or import your JSON/`PB2.` code. Permission for the old path does not automatically allow the new executable.

When moving the application, add its new path in HidHide. To stop hiding the wheel, clear **Enable device hiding** and reconnect it. To remove only the application, stop it, back up your profile and delete its folder. ViGEmBus and HidHide are separate installations; remove them through Windows only if desired and no other application depends on them.
