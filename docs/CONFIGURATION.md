# Configuration and calibration

**English** | [Português (Brasil)](pt-BR/CONFIGURATION.md)

[README](../README.md) · [Profiles](PROFILES.md) · [Field reference](API.md)

Valid changes are saved automatically in `profile.ini`. Wait for the save status before stopping the process. Browser adjustments are sent to the native core; C computes the output used by the game.

## Overview and modes

- **Device:** reads hardware. Status can be **Input detected**, **USB detected** without readable input, or **Waiting for USB**.
- **Demo:** synthetic inputs with game output blocked. **Use keyboard and mouse** switches automatic playback to interaction.
- **Enable bridge / Disable bridge:** requests or removes the virtual controller on Windows. Check the actual result in Diagnostics.
- **Light/Dark theme:** a browser preference stored separately from the profile.
- **Language:** English first, **Português (Brasil)** second. The choice is remembered separately from the profile and does not change input/output settings.

The overview wheel, pedals, LX/RT/LT and sticks show **calibrated output**. The ±135° graphic is a 270° visual scale, not measured hardware degrees or a change to physical travel. Button lights on the wheel show inputs; mapper cards indicate outputs.

The displayed rate is observed in the C loop. The microsecond timing measures only input-to-report mapping, not total wheel-to-game latency.

### Explore Demo

Click **Use keyboard and mouse**, move focus away from text fields/sliders, and use:

| Control | Keys |
| --- | --- |
| Steering | A / D |
| Throttle / brake | W / S |
| Controller A / B / X / Y | J / K / U / I |
| LB / RB | Q / E |
| D-pad | Arrow keys |

Sliders also simulate steering, pedals and other sticks. The mapper does not capture real keyboard/mouse inputs in physical mode; these keys belong to Demo only.

## Mapping: start from the game's output

Each card represents **one control the game receives**. Choose its physical source under **Receive movement from** or **Receive from**. One input can drive multiple outputs.

| Output | Default role |
| --- | --- |
| LX | Steering, −100% to +100% |
| LY | Left stick vertical |
| RX / RY | Right stick horizontal / vertical |
| RT | Throttle, 0 to 100% |
| LT | Brake, 0 to 100% |
| Buttons/D-pad | D-pad, Start, Back, LS, RS, LB, RB, A, B, X, Y |

### Select or identify a source

Choose from normalized axes, buttons/D-pad and six raw axes exposed by the backend. A listed control may be absent from your wheel. **Disabled** keeps that output neutral.

For **Identify**, start at rest, click the card's button and move only the desired input. If noise identifies another input, stabilize the device and try again, or select the source manually.

**Swap throttle and brake** swaps the RT/LT sources. Calibration stays with each output; check it after swapping. This button does not fix a pedal that reads 100% when released: check **Invert direction** for that.

### Buttons driving an analog stick

For LX/LY/RX/RY, choose a positive button and a second source under **Use two inputs for both directions**. For example, D-pad right as positive and left as negative can drive LX. Pressing both cancels movement. A single button or pedal source starts at zero and moves toward the positive side.

### Axis or pedal driving a button

Choose an axis/pedal on a button card and open **Axis / pedal threshold**. Select the positive or negative direction and activation percentage. For example, negative RX at 60% activates the button below −0.60. The threshold uses the **source input**, not another output axis's calibration.

A physical button produces 0 or 1 and normally uses **Positive direction / pressed**. This output is intentionally digital; RT/LT remain progressive when driven by axes/pedals.

## Per-axis calibration

![Light calibration view](calibration-light.png)

Set sources in Mapping first. The main Calibration cards are steering, throttle and brake. Open **Calibrate the other sticks** for LY/RX/RY. Each output has its own settings.

| Setting | Effect | Range |
| --- | --- | --- |
| Deadzone | Ignores jitter near rest | 0–60% |
| Reach 100% output at | Movement needed to saturate output | 5–100% |
| Response curve | Distributes sensitivity across useful travel | 0.2–3; default 1 |
| Invert direction | Reverses the selected source | on/off |
| Center | Compensates for an offset rest position | −50% to +50% |

Full travel must exceed the deadzone by **at least 2 percentage points**. Inconsistent values are rejected.

### Steering, step by step

1. Leave the wheel straight and released. Use **Capture center** near rest. Captures outside ±50% are rejected.
2. Click **Measure rest** and leave it released for about 3 seconds. The interface estimates a deadzone with a small margin; check the result.
3. Adjust the orange arc handles to cover only noise. Excessive deadzone discards useful small movements.
4. Adjust the blue handles or **Reach 100% output at**. At 50%, half the normalized movement already gives maximum steering.
5. Start with curve **1**. Below 1 responds earlier; above 1 is gentler near rest.
6. Check left, center and right in both input and output, then in the game, which can add its own curve/deadzone.

**Capture maximum** uses the current movement: hold the wheel at the position you want to represent 100% when clicking. It does not record a full sweep automatically.

### Pedals, step by step

1. Release both pedals and check for rest near zero. If swapped, use **Swap throttle and brake**.
2. If input decreases when pressing, check the source and **Invert direction**. An inverted unipolar pedal source driving a pedal output uses `1 − input`; signed axes invert their sign.
3. Set the deadzone using the orange bar base or **Measure rest**.
4. Press one pedal to the desired position and use **Capture maximum**, or adjust the blue limit/number.
5. Repeat for the other pedal. Test both together in XInput.

A signed axis driving RT/LT uses its positive side; inversion selects the negative side. Pedal center exists in the technical format, but the interface does not expose pedal center capture/numeric controls.

### How the calculation works

After source selection, negative-input combination, inversion and center compensation, useful travel is renormalized:

```text
magnitude = clamp((abs(input) − deadzone) / (full_travel − deadzone), 0, 1)
output = magnitude ^ curve  (preserving sign for signed outputs)
```

Example: 10% deadzone, 50% full travel, curve 1 and 30% input produce 50% output. Output is zero inside the deadzone and 100% above the limit. There is no hidden digital pedal threshold.

Graphs show selected input after center/inversion and calculated output. Orange marks ignored movement; blue marks saturation. During a drag, graphics update immediately and settings are sent at a minimum interval of 120 ms and on release. Wait for saving before exporting or stopping.

## Diagnostics: source and rate

| Option | When to change it |
| --- | --- |
| Physical XInput source | Automatic prefers wheel subtype; select slot 0–3 if the driver presents the wheel as a gamepad. Recheck after reconnecting devices. |
| Force DirectInput | When XInput cannot provide input. Pedals will be combined. |
| Target C loop rate | 125, 250 (default), 500 or 1000 Hz. Raising it can increase CPU use without adding USB samples. |
| Steering axis · DirectInput | X, Y, Z, RX, RY or RZ; default X. |
| Combined pedal axis · DirectInput | The same six axes; default Z, negative throttle and positive brake. |

XInput/DirectInput options are Windows-specific; Linux uses evdev/xpad. Raw axes and the reported backend help identify the correct source.

**Export diagnostics** downloads `boreal-wheel-diagnostics.json` containing the current state, report, settings, backend and timestamp. This is a diagnostic snapshot, **not an importable profile**. Exported data uses stable English field names in both interface languages.

## Restore defaults

Under **Mapping → Your profile, in one code**, click **Restore defaults**, then **Confirm reset** within 5 seconds. This replaces settings and saves defaults. [Export a backup first](PROFILES.md).

Reset and import do not automatically disable an already active bridge. Disable it before changing a large set of controls, then enable it after checking the result. Theme and language are independent of profile reset.
