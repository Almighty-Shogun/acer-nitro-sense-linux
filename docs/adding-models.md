# Adding A Model

Model support is added through a JSON profile under `models/`. Do not copy a
profile blindly between laptops unless the EC registers and behavior are
validated on the target hardware.

## 1. Identify The Laptop

Collect DMI data:

```sh
cat /sys/class/dmi/id/product_name
cat /sys/class/dmi/id/product_version
cat /sys/class/dmi/id/board_name
cat /sys/class/dmi/id/bios_version
```

The profile must include matching DMI allow-list values so the daemon refuses to
run on unrelated machines by default.

## 2. Start From The Closest Profile

Copy the closest known Nitro model:

```sh
cp models/acer-nitro-an517-51.json models/acer-nitro-YOUR-MODEL.json
```

Update at least:

- `id`
- `model`
- DMI allow-list values
- fan RPM registers
- fan speed write registers
- temperature sensor settings
- presets and safety thresholds after validation

## 3. Validate JSON Syntax

The parser intentionally accepts a constrained JSON subset. Validate before
installing:

```sh
build/acer-nitro-sensed --config models/acer-nitro-YOUR-MODEL.json --force-model --check-config
```

If `--config` is supplied explicitly, invalid profiles fail instead of falling
back to another bundled model.

## 4. Probe EC Access

Confirm the daemon can open an EC backend:

```sh
sudo build/acer-nitro-sensed --config models/acer-nitro-YOUR-MODEL.json --force-model --probe
```

The output should show the selected backend and readable fan/temperature
values. Do not continue to fan writes until read registers look plausible.

## 5. Validate Fan RPM And Writes

Install the profile locally, then run hardware validation:

```sh
scripts/validate-hardware.sh
```

For fan percentage calibration:

```sh
CALIBRATE_FANS=1 scripts/validate-hardware.sh
```

Useful calibration options:

```sh
CALIBRATE_FANS=1 CALIBRATION_SPEEDS='10 20 30 40 50 60 70 80 90 100' scripts/validate-hardware.sh
CALIBRATE_FANS=1 EC_DUMP=1 scripts/validate-hardware.sh
```

Calibration should confirm:

- CPU fan RPM changes when CPU fan speed is written.
- GPU fan RPM changes when GPU fan speed is written.
- Firmware auto/manual/turbo values are correct if the model declares fan modes.
- Reset writes return the machine to a reasonable idle state.

## 6. Validate Temperature Sources

Prefer Linux hwmon sensors where they are stable. If the GPU hwmon path becomes
unreadable while the dGPU is runtime-suspended, validate a model-specific EC
temperature register before adding `temperature_register`.

Read candidate EC registers only after confirming EC access:

```sh
ans ec read 0x2a
ans ec dump 0x10 0x60
```

Temperature bytes should track real temperature changes. Do not assume the
AN517-51 GPU register `0x2a` is universal.

## 7. Validate Keyboard Backlight

First check whether Linux exposes a standard keyboard backlight LED:

```sh
ls /sys/class/leds
```

If no usable LED exists, use the discovery helper:

```sh
sudo scripts/discover-keyboard-backlight.sh
```

Only add EC keyboard backlight registers after confirming visible brightness
steps and safe restore behavior on the target model.

## 8. Tune Presets And Safety

Do not tune presets from another model's fan curve. Use the calibration table
and daily observation runs:

```sh
DAILY_MODE=1 SAMPLES=120 INTERVAL=5 scripts/validate-hardware.sh
```

A good profile should:

- boot quietly in the default mode
- avoid full fan speed at the login screen
- keep idle temperatures stable
- make safety overrides visible in status
- restore firmware/EC state after validation

## 9. Submit The Model

Include:

- the new JSON profile
- `--check-config` output
- `--probe` output
- validation summary path or summary text
- DMI values
- notes about EC access backend and kernel

Do not include private local logs containing usernames, hostnames, or unrelated
hardware details unless they are needed for review.
