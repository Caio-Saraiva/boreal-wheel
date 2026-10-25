# Profiles, export, import and backups

**English** | [Português (Brasil)](pt-BR/PROFILES.md)

[README](../README.md) · [Configuration](CONFIGURATION.md) · [Technical format](API.md)

## What is saved

The application keeps **one active profile per working folder**, automatically saved as `profile.ini`. There is no named profile library inside the interface: export separate files for different games and import the one you want.

| Included in a profile | Stored separately |
| --- | --- |
| Sources for six axes and 14 buttons/D-pad directions | Light/dark theme and language in browser local storage |
| Per-axis deadzone, limit, curve, inversion and center | HidHide allowed executables and hidden devices |
| Negative source for four sticks | ViGEmBus and physical-driver installations |
| Button direction and threshold | In-game settings |
| XInput slot, DirectInput, DI axes and target rate | Demo mode, momentary input and bridge activation |

At startup, the application loads `profile.ini`, but **the bridge starts disabled**. Importing preserves the bridge's current state; if already active, changes can reach the game. Disable it before replacing a set of controls.

Language changes never translate JSON keys, numeric values, profile codes or filenames. English and Portuguese interfaces share the same interoperable profile format.

## Automatic saving

1. Make a valid change in the interface.
2. Wait for the save status. There is no general Save button.
3. The native core writes `profile.ini.tmp` and replaces `profile.ini` after the write succeeds.

The file is in the process's **working directory**, which is not necessarily the executable's folder. Supplied scripts choose these locations:

| Launch method | INI location |
| --- | --- |
| Packaged `run.cmd` or localized launchers | Extracted folder beside the executable |
| `run-windows.ps1` | `build-windows/Release/profile.ini` |
| `./run.sh` | Repository root |
| Executable called directly | Working directory selected by the terminal/shortcut |

If **Cannot save profile.ini** appears, use a writable folder. Avoid running from the ZIP's temporary extraction or a protected folder such as `Program Files` without write permission.

## Export as a code

1. Open **Mapping → Your profile, in one code**.
2. Wait for pending settings and click **Generate code**.
3. Click **Copy code**. If clipboard access is blocked, select the whole field and press `Ctrl+C`.
4. Save the complete code starting with **`BW2.`** in a note/text file. Use a descriptive name, such as `smooth-steering-2026-10-25`.

The code is Base64-encoded JSON, not encryption, and requires no internet. It carries the same settings as the JSON file. Keep the prefix and the complete contents. If you adjust settings later, click **Generate code** again: the text already in the field is a snapshot of when it was generated.

## Import a code

1. Disable the bridge if playing.
2. Export current settings so you can go back.
3. Paste the complete code into **Profile code**.
4. Click **Apply profile** and wait for **Profile applied and saved**.
5. Check sources and calibration. An XInput slot can refer to another device on another PC.
6. Enable the bridge when ready.

The field also accepts complete JSON. Legacy **`PB2.`** codes and JSON with `"format": "project-b"`, version 2, remain supported. New exports use `BW2.` and `"format": "boreal-wheel"`. The older application is not guaranteed to accept the new format.

## Export a JSON file

Click **Download file** in the same area. The browser saves `boreal-wheel-profile.json` to its configured downloads folder. Rename it to something useful, such as `my-game-458-italia.json`.

For several profiles:

```text
BorealProfiles/
  linear-racing.json
  sensitive-racing.json
  backup-before-update.json
```

Downloading does not change the active profile's name or location. Exports are independent backups; later adjustments do not update them.

## Import a JSON file

1. Click **Open file**.
2. Select an exported JSON file. Selection starts import immediately; no extra Apply profile click is needed.
3. Wait for confirmation and check your settings.

The interface accepts files up to 40,000 bytes. The version and every field must be valid. Diagnostic files, INI files and screenshots are not JSON profiles.

Incomplete imports, unknown fields, out-of-range values and incompatible versions are rejected before replacing the saved profile. On error, previous settings stay intact. See the [technical reference](API.md) to inspect the format, but prefer editing through the interface.

## Back up profile.ini directly

Stop the process and copy `profile.ini` elsewhere. To restore, stop the application, place the copy in the correct working directory and start again. Do not edit the INI while the process runs: the next interface change may overwrite it.

INI is internal storage. It accepts some legacy keys but does not use the JSON envelope validation. Loading an INI ignores lines/keys it cannot parse; use complete validated JSON/code exports when transferring between people or PCs.

## Update or migrate from Project B

1. Export a JSON/code from the old application and back up its INI.
2. Close the game and stop the old process.
3. Extract the new distribution and copy the INI to the new folder, or import the old file/code through the interface.
4. In HidHide, allow the **new full path to `boreal-wheel.exe`**. This is required even when only the executable name changed.
5. Open the application, use `Ctrl+F5`, and check input, sources, center and pedal travel before enabling the bridge.

An older theme preference is reused if available at the same browser origin. Changing port, browser, browser profile or clearing browser data can reset the theme/language preference; it does not delete the on-disk INI. Profile format version 2 remains unchanged in application version 0.3.0.

## Share settings and restore defaults

Share only the profile JSON or code. It does not contain GitHub credentials, Git name/email or HidHide paths. Diagnostics can contain device identification and environment details; inspect that file before sharing it for help.

Use **Restore defaults → Confirm reset** to return to defaults. To experiment without deleting settings, stop the application and move the INI to a backup: it starts with defaults when none exists. The [example profile](../examples/default-profile.json) also contains current defaults without hardware-specific calibration.
