# Safety Behavior

Fan control commands pass through the daemon safety layer before writing EC
registers. Safety rules apply to automatic curves, presets, restored state, and
manual `set` commands.

Automatic curve mode should be the normal daily mode. Manual percentages and
fixed presets intentionally hold the selected speed until changed; they do not
track rising temperature except through safety clamps.

`firmware-auto` leaves normal fan-speed control to the Acer firmware, but safety
still applies. When a safety condition activates in firmware-auto mode, the
daemon temporarily switches the EC fan-mode registers to manual, writes the
safety speed, and restores firmware-auto fan mode after safety clears. During
that override, status reports the affected fans with `control=safety` instead
of `control=firmware`.

## Model Fields

Safety is configured inside the model `ec` block:

```json
{
  "safety": {
    "min_speed_percent": 30,
    "min_speed_temperature_c": 70,
    "critical_speed_percent": 100,
    "auto_ramp_up_percent": 12,
    "auto_ramp_bypass_temperature_c": 0,
    "missing_temperature_speed_percent": 60,
    "max_ec_read_failures": 3,
    "max_ec_write_failures": 3
  }
}
```

Fields:

- `min_speed_percent`: minimum fan speed allowed once temperature reaches
  `min_speed_temperature_c`.
- `min_speed_temperature_c`: temperature where minimum speed enforcement begins.
- `critical_speed_percent`: speed used at or above `critical_temperature_c`.
- `auto_ramp_up_percent`: maximum upward percentage step per daemon poll while
  automatic curve mode is tracking normal temperatures. `0` disables ramping.
- `auto_ramp_bypass_temperature_c`: temperature where automatic mode stops
  ramp-limiting and writes the curve target immediately. `0` disables this
  temperature-only bypass; critical safety still bypasses the ramp limiter.
- `missing_temperature_speed_percent`: minimum speed when sensor temperature is
  unavailable. Individual fan objects can override this with their own
  `missing_temperature_speed_percent`.
- `max_ec_read_failures`: repeated EC read failures before forcing critical
  speed.
- `max_ec_write_failures`: repeated EC write failures before reporting degraded
  write state and requesting critical speed on later attempts.

## Status

Each fan in `/run/acer-nitro-sense/status.json` includes:

```json
{
  "control": "firmware",
  "active_percent": null,
  "firmware_controlled": true,
  "critical_temp_samples": 0,
  "ec_read_failures": 0,
  "ec_write_failures": 0,
  "safety_active": false,
  "safety_reason": ""
}
```

`active_percent` is `null` while firmware owns fan speed because the daemon
cannot know the firmware's internal percentage decision. During daemon or safety
control it is the percentage written by the daemon.
`critical_temp_samples` shows how many consecutive accepted control temperature
samples are at or above the model's critical threshold.

Reasons:

- `temperature-unknown`
- `minimum-safe-speed`
- `critical-temperature`
- `ec-read-failure`
- `ec-write-failure`

Critical temperature and repeated EC failures are global safety conditions. When
one fan/component reaches one of those conditions, both fans are requested at
critical speed.

Missing temperatures are handled per fan. If one fan has no temperature, only
that fan is clamped to its missing-temperature fallback unless another global
safety condition is active.

## Temperature Spikes

Some AN517-51 readings can jump upward by 15°C or more for a single sample and
then immediately return to the previous range. The daemon filters those
one-sample upward spikes before they affect status display, automatic curves, or
critical safety. If the high value appears again on the next poll, it is
accepted as real.

At daemon startup, cached temperatures from the previous status/cache file are
used as the trusted baseline when available. If no trusted baseline exists, a
single first critical-temperature reading is treated as pending until the next
poll. This avoids a full-speed startup blast from one unstable sensor read while
still accepting repeated critical readings.

JSON status keeps both values:

- `temp_c`: filtered component temperature used by status and the GNOME
  extension.
- `sensor_temp_c`: raw sensor temperature from Linux hwmon.
- `control_temp_c`: filtered temperature used for the fan curve and safety
  decisions. On AN517-51 both fans use the hotter CPU/GPU temperature for
  control while still reporting their own component `temp_c`.
- `control_sensor_temp_c`: raw sensor value behind `control_temp_c`.
- Spike logs include `source=component:<group>` or `source=control:<group>` so
  shared control groups such as `max` are not mistaken for a specific GPU or CPU
  component reading.
