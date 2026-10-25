# API and profile format reference

**English** | [Português (Brasil)](pt-BR/API.md)

[README](../README.md) · [Using profiles](PROFILES.md) · [Development](DEVELOPMENT.md)

Reference for application 0.3.0 and profile format version 2. Use the interface for ordinary configuration. This local API serves the browser; it does not provide arbitrary device discovery or authenticated remote access.

## Address and requests

Default: `http://127.0.0.1:4580`. The process listens only on loopback. `--port N` accepts 1024–65535; use the same port in Host and Origin.

- GET returns state, profile or explicitly allowed static resources.
- POST requires exactly `Origin: http://127.0.0.1:4580` and `Content-Type: application/x-www-form-urlencoded`.
- Bodies contain numeric key/value pairs, or `action=name`. Do not send JSON as the POST body.
- Host must be `127.0.0.1:port`. `localhost`, external origins, duplicate setting keys, chunked transfer and ambiguous lengths are rejected.
- The browser limits imported text/files to 40,000 characters/bytes respectively. Native declared HTTP body length is limited to 16,384 bytes and must fit the request buffer.

Read a profile without changing anything:

```bash
curl http://127.0.0.1:4580/api/profile
```

Apply and save a partial setting change:

```bash
curl -X POST http://127.0.0.1:4580/api/config \
  -H 'Origin: http://127.0.0.1:4580' \
  -H 'Content-Type: application/x-www-form-urlencoded' \
  --data 'axis_deadzone_0=0.03&axis_limit_0=0.80'
```

In PowerShell, use `curl.exe` and its appropriate line-continuation syntax, or put the command on one line.

## Routes

| Method and path | Result |
| --- | --- |
| `GET /api/state` | Input, output, settings and diagnostics snapshot |
| `GET /api/profile` | JSON envelope containing the 87 current configuration fields |
| `POST /api/config` | Partial update, validated and saved before replacing settings |
| `POST /api/profile` | Complete replacement; requires exactly the 87 allowed fields |
| `POST /api/action` | Mode, bridge, reset or capture action |
| `POST /api/demo` | Manual synthetic state: steering/ly/rx/ry −1 to 1; throttle/brake 0 to 1; integer buttons 0 to 65535 |
| `GET /`, `/app.js`, `/i18n.js`, `/locales.js`, `/style.css`, `/favicon.svg` | Local interface resources |

`/api/demo` replaces a complete manual-input snapshot: omitted fields return to zero. It feeds input only in manual Demo mode and cannot create game output. The native parser also accepts `raw_0`–`raw_5`, but the Demo worker regenerates the raw-axis snapshot from its normalized controls; these keys do not provide persistent independent raw-axis simulation.

Successful POST returns `{"ok":true}`. Errors are text: 400 for invalid settings/actions or save failure, 403 for invalid Host/origin/type, and 404 for unrecognized routes. There is no arbitrary disk-file endpoint or Markdown documentation server.

### Actions

Send `action=name` to `/api/action`:

| Name | Effect |
| --- | --- |
| `physical` | Select hardware and clear the bridge request |
| `demo` | Select automatic Demo and clear the bridge request |
| `manual` | Select manual Demo input and reset synthetic state |
| `auto` | Resume automatic Demo animation |
| `start` / `stop` | Request bridge activation/deactivation; start is rejected in Demo |
| `reset` | Save and apply defaults; keep current bridge state |
| `center` or `center_0` through `center_3` | Capture selected-axis center; requires input and center within ±0.5 |
| `limit_0` through `limit_5` | Capture current magnitude as full travel; requires input, at least 0.05, and 0.02 separation from deadzone |

**Measure rest** and **Identify** are browser operations that observe snapshots and submit settings, rather than dedicated endpoints.

### State snapshot

`/api/state` contains:

- `demo`, `demo_auto`, `connected`, `physical_connected`, `usb_present`, `combined_pedals`: 0/1 indicators. Demo can report connected=1 without hardware; physical_connected distinguishes this.
- `output_requested` and `output_active`: requested and actual output state, which can differ without input/a driver.
- `platform`, `name`, `backend`, `detail`, `output_status`: identification and messages.
- `map_us`, `observed_hz`, `packets`: transformation duration, observed loop rate and iteration count. packets does not guarantee new USB packets.
- `raw`: steering, pedals, LY/RX/RY, button bitmask and six backend axes.
- `input`: six selected inputs after center/inversion and before curve/deadzone.
- `report`: integer LX/LY/RX/RY from −32768 to 32767, LT/RT from 0 to 255 and a button bitmask.
- `config`: current configuration, including legacy compatibility aliases. Do not POST all aliases to `/api/profile`; obtain the exportable set with `GET /api/profile`.

### Language contract

API routes, JSON keys, configuration identifiers, numeric values, action names and profile format are **language-independent**. Native human-readable messages default to English. Start with `--lang pt-BR` or `BW_LANG=pt-BR` to use Portuguese messages; explicit `--lang` overrides the environment.

The interface translates supported messages to its separately selected language. Human-readable strings are diagnostic text, not stable programmatic identifiers; automate using fields such as `output_active` rather than matching translated sentences. No language field is added to the 87-field wheel profile.

## Exported envelope

The [complete example](../examples/default-profile.json) contains all defaults. General shape:

```json
{
  "format": "boreal-wheel",
  "version": 2,
  "settings": {
    "poll_hz": 250,
    "slot": -1
  }
}
```

**This snippet is illustrative and incomplete; it cannot be imported.** `settings` must have exactly 87 fields: five global, 40 axis and 42 button fields. Values are finite JSON numbers, with integers where required. JSON does not allow comments.

Portable code: `BW2.` followed by Base64 of the JSON text. The interface also accepts legacy `PB2.` codes and `project-b` format with version 2. It validates the envelope and sends only `settings` as form pairs to the core. Native field, range and consistency validation occurs before saving/replacing; invalid imports preserve the previous profile.

### Global fields

| Field | Default | Values |
| --- | --- | --- |
| `poll_hz` | 250 | 125, 250, 500 or 1000 |
| `slot` | −1 | −1 automatic or physical XInput 0–3 |
| `directinput` | 0 | 0 automatic/XInput; 1 force DI |
| `steering_axis` | 0 | 0–5: X, Y, Z, RX, RY, RZ in DI |
| `pedal_axis` | 2 | 0–5: combined pedal axis in DI |

### Axis fields

Index `i`: **0=LX/steering, 1=LY, 2=RX, 3=RY, 4=RT/throttle, 5=LT/brake**.

| Field for each i | Default | Range |
| --- | --- | --- |
| `axis_source_i` | i | −1 or a valid source below |
| `axis_invert_i` | 0 | 0/1 |
| `axis_deadzone_i` | 0.02 for sticks; 0.01 for pedals | 0–0.6 |
| `axis_limit_i` | 1 | 0.05–1 |
| `axis_gamma_i` | 1 | 0.2–3 |
| `axis_center_i` | 0 | −0.5–0.5 |
| `axis_negative_i` | −1 | Only i=0–3; −1 or valid source |

`axis_limit_i − axis_deadzone_i` must be at least 0.02, subject to internal floating-point tolerance. Do not send `axis_negative_4` or `_5`. An output with source −1 stays neutral even if inverted.

### Sources

| ID | Input |
| --- | --- |
| −1 | Disabled |
| 0, 1, 2, 3 | Normalized steering/LX, LY, RX, RY |
| 4, 5 | Normalized throttle/RT and brake/LT |
| 6–15 | D-pad up, down, left, right, Start, Back, LS, RS, LB, RB |
| 16, 17 | Reserved; invalid as sources |
| 18–21 | A, B, X, Y |
| 22–27 | Six raw backend axes in raw.axes order |

Raw axes depend on the backend. Here “raw” means normalized driver readings, not USB bytes. Selecting a listed control does not add sensors absent from the wheel.

### Button fields

Index `b` is the XInput bit. Valid indices are 0–9 and 12–15. Bits 10 and 11 are reserved and excluded from the profile.

| Bit b | Output |
| --- | --- |
| 0, 1, 2, 3 | D-pad up, down, left, right |
| 4, 5 | Start, Back |
| 6, 7 | LS, RS |
| 8, 9 | LB, RB |
| 12, 13, 14, 15 | A, B, X, Y |

For every valid bit:

| Field | Default | Values |
| --- | --- | --- |
| `button_source_b` | 6+b | −1 or valid source |
| `button_direction_b` | 1 | −1 or 1 |
| `button_threshold_b` | 0.5 | 0.01–1 |

A button activates when `source × direction >= threshold`. source −1 stays off. Thresholds use the original source, independently of another output axis's calibration.

## INI storage and compatibility

`profile.ini` uses `key=value` without an envelope. Temporary-file writing and replacement reduce partial-profile risk. Paths are relative to the working directory; see [profile locations](PROFILES.md).

The reader accepts legacy aliases including `deadzone`, `gamma`, `center`, `pedal_deadzone`, `invert_steering`, `invert_throttle`, `invert_brake` and `button_b`. They support INI migration and partial updates; complete exportable profiles use only current fields. INI loading ignores unrecognized lines and applies the candidate only if axis-limit consistency holds. It is not the strict interface-import contract.

Theme, language, HidHide rules and the bridge request are outside the profile format. Keep credentials and personal configuration out of the repository.
