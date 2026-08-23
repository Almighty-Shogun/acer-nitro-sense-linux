#!/usr/bin/env sh
set -eu

ROOT_DIR="$(CDPATH='' cd "$(dirname "$0")/.." && pwd)"
OUT_DIR="$(mktemp -d)"
trap 'rm -rf "$OUT_DIR"' EXIT INT TERM

export OUT_DIR
export SAMPLES=2
export INTERVAL=1
export LOAD_CMD=
export COOLDOWN_SECONDS=0
export RESTORE_COOLDOWN_SECONDS=0
export MODE_MATRIX=0
export DAILY_MODE=1
export CALIBRATE_FANS=0
export CALIBRATION_TARGETS=all
export CALIBRATION_SPEEDS=50
export CALIBRATION_FAN_MODE=manual
export CALIBRATION_PROFILE=quiet
export CALIBRATION_SETTLE_SECONDS=1
export CALIBRATION_SAMPLES=1
export EC_DUMP=0
export EC_DUMP_START=0x10
export EC_DUMP_END=0x20
export FAN_MODE_SCAN=0
export FAN_MODE_SCAN_MODES=auto
export FAN_MODE_SCAN_PROFILES=quiet
export SAFETY_VALIDATE=0
export SAFETY_VALIDATE_MODES=manual
export SAFETY_VALIDATE_PERCENT=25
export SAFETY_VALIDATE_PRESET=quiet
export SAFETY_VALIDATE_SETTLE_SECONDS=1
export INITIAL_COOLBOOST=off
export INITIAL_FAN_MODE=auto
export INITIAL_PROFILE=quiet
export INITIAL_POWER_SOURCE=ac
export INITIAL_CONTROL_MODE=firmware-auto
export INITIAL_PRESET=firmware-auto
export INITIAL_CPU_REQUESTED=55
export INITIAL_GPU_REQUESTED=52

. "$ROOT_DIR/scripts/hardware/lib/validate-logging.sh"
. "$ROOT_DIR/scripts/hardware/lib/validate-summary-features.sh"
. "$ROOT_DIR/scripts/hardware/lib/validate-summary-status.sh"
. "$ROOT_DIR/scripts/hardware/lib/validate-summary-calibration.sh"
. "$ROOT_DIR/scripts/hardware/lib/validate-summary.sh"

cat > "$OUT_DIR/daily-observation.status.log" <<'EOF'
# label=daily-observation
cpu rpm=3000 temp=60C requested=55 effective=55 percent=55 write_value=55 safety=ok critical_samples=0
gpu rpm=2800 temp=52C requested=52 effective=52 percent=52 write_value=52 safety=ok critical_samples=0
control fan=cpu control_temp=60 control_sensor_temp=60
control fan=gpu control_temp=60 control_sensor_temp=60
EOF

cat > "$OUT_DIR/final-status.txt" <<'EOF'
model=Acer Nitro 5 AN517-51
EOF

write_summary >/dev/null

grep -q 'daily-observation cpu_avg_rpm=3000' "$OUT_DIR/summary.txt"
grep -q 'gpu_avg_temp=52.0C' "$OUT_DIR/summary.txt"
grep -q 'Final status:' "$OUT_DIR/summary.txt"
! grep -q '^+ ' "$OUT_DIR/summary.txt"
