#!/usr/bin/env sh
set -eu

SCRIPT_DIR="$(CDPATH= cd "$(dirname "$0")" && pwd)"

. "$SCRIPT_DIR/lib/validate-defaults.sh"
. "$SCRIPT_DIR/lib/validate-logging.sh"
. "$SCRIPT_DIR/lib/validate-runtime.sh"
. "$SCRIPT_DIR/lib/validate-summary-features.sh"
. "$SCRIPT_DIR/lib/validate-summary-status.sh"
. "$SCRIPT_DIR/lib/validate-summary-calibration.sh"
. "$SCRIPT_DIR/lib/validate-summary.sh"
. "$SCRIPT_DIR/lib/validate-phases.sh"
. "$SCRIPT_DIR/lib/validate-calibration.sh"
. "$SCRIPT_DIR/lib/validate-scan.sh"
. "$SCRIPT_DIR/lib/validate-safety.sh"

mkdir -p "$OUT_DIR"
INITIAL_COOLBOOST="$(coolboost_state || true)"
INITIAL_FAN_MODE="$(fan_mode_state || true)"
INITIAL_PROFILE="$(profile_state || true)"
INITIAL_POWER_SOURCE="$(power_source_state || true)"
INITIAL_GPU_TEMP_POLICY="$(gpu_temp_policy_state || true)"
INITIAL_CONTROL_MODE="$(control_mode_state || true)"
INITIAL_PRESET="$(preset_state || true)"
INITIAL_CPU_REQUESTED="$(fan_requested_state cpu || true)"
INITIAL_GPU_REQUESTED="$(fan_requested_state gpu || true)"
if [ "$DAILY_MODE" != "1" ] || [ "$CALIBRATE_FANS" = "1" ] || [ "$FAN_MODE_SCAN" = "1" ] || [ "$SAFETY_VALIDATE" = "1" ]; then
    trap restore_controls EXIT INT TERM
fi

progress "writing validation logs to $OUT_DIR"

cat > "$OUT_DIR/README.txt" <<EOF
Acer Nitro Sense hardware validation

SAMPLES=$SAMPLES
INTERVAL=$INTERVAL
LOAD_CMD=$LOAD_CMD
LOAD_WORKERS=$LOAD_WORKERS
COOLDOWN_SECONDS=$COOLDOWN_SECONDS
RESTORE_COOLDOWN_SECONDS=$RESTORE_COOLDOWN_SECONDS
MODE_MATRIX=$MODE_MATRIX
DAILY_MODE=$DAILY_MODE
CALIBRATE_FANS=$CALIBRATE_FANS
CALIBRATION_TARGETS=$CALIBRATION_TARGETS
CALIBRATION_SPEEDS=$CALIBRATION_SPEEDS
CALIBRATION_FAN_MODE=$CALIBRATION_FAN_MODE
CALIBRATION_PROFILE=$CALIBRATION_PROFILE
CALIBRATION_SETTLE_SECONDS=$CALIBRATION_SETTLE_SECONDS
CALIBRATION_SAMPLES=$CALIBRATION_SAMPLES
EC_DUMP=$EC_DUMP
EC_DUMP_START=$EC_DUMP_START
EC_DUMP_END=$EC_DUMP_END
FAN_MODE_SCAN=$FAN_MODE_SCAN
FAN_MODE_SCAN_MODES=$FAN_MODE_SCAN_MODES
FAN_MODE_SCAN_PROFILES=$FAN_MODE_SCAN_PROFILES
FAN_MODE_SCAN_SETTLE_SECONDS=$FAN_MODE_SCAN_SETTLE_SECONDS
SAFETY_VALIDATE=$SAFETY_VALIDATE
SAFETY_VALIDATE_MODES=$SAFETY_VALIDATE_MODES
SAFETY_VALIDATE_PERCENT=$SAFETY_VALIDATE_PERCENT
SAFETY_VALIDATE_PRESET=$SAFETY_VALIDATE_PRESET
SAFETY_VALIDATE_SETTLE_SECONDS=$SAFETY_VALIDATE_SETTLE_SECONDS
initial_coolboost=${INITIAL_COOLBOOST:-unknown}
initial_fan_mode=${INITIAL_FAN_MODE:-unknown}
initial_profile=${INITIAL_PROFILE:-unknown}
initial_power_source=${INITIAL_POWER_SOURCE:-unknown}
initial_gpu_temp_policy=${INITIAL_GPU_TEMP_POLICY:-unknown}
initial_control_mode=${INITIAL_CONTROL_MODE:-unknown}
initial_preset=${INITIAL_PRESET:-unknown}
initial_cpu_requested=${INITIAL_CPU_REQUESTED:-unknown}
initial_gpu_requested=${INITIAL_GPU_REQUESTED:-unknown}
validation_started_at=$VALIDATION_STARTED_AT

This script logs fan RPM, temperatures, mode, preset, and CoolBoost state.
It can toggle CoolBoost, fan modes, and platform profiles. On exit it always
turns CoolBoost off and leaves fan mode in a non-turbo state.
EOF

run_log uname uname -a
run_log doctor "$ANS" doctor
run_log status-json "$ANS" status --json
run_log capabilities "$ANS" capabilities
run_log power-source "$ANS" power-source status
run_log gpu-temp "$ANS" gpu-temp status
run_log keyboard-backlight "$ANS" keyboard-backlight status
collect_current_daemon_journal journal-current-daemon
run_log journal journalctl -u acer-nitro-sense.service -n 120 --no-pager
run_log feature-power sh -c 'command -v powerprofilesctl >/dev/null && powerprofilesctl 2>&1 || echo powerprofilesctl=unavailable; systemctl --no-pager --full status power-profiles-daemon.service 2>&1 || true; ls -l /sys/firmware/acpi/platform_profile /sys/firmware/acpi/platform_profile_choices 2>&1; cat /sys/firmware/acpi/platform_profile /sys/firmware/acpi/platform_profile_choices 2>&1'
run_log feature-power-supply sh -c 'for p in /sys/class/power_supply/*; do [ -e "$p" ] || continue; echo "$p"; for f in type online status capacity charge_now charge_full energy_now energy_full power_now voltage_now manufacturer model_name; do [ -r "$p/$f" ] && printf "  %s=" "$f" && cat "$p/$f" 2>&1; done; done'
run_log feature-keyboard-backlight sh -c 'acer-nitro-sense keyboard-backlight status 2>&1; ls -l /sys/class/leds 2>&1; for l in /sys/class/leds/*; do [ -e "$l" ] || continue; echo "$l"; for f in brightness max_brightness trigger delay_on delay_off; do [ -r "$l/$f" ] && printf "  %s=" "$f" && cat "$l/$f" 2>&1; done; [ -e "$l/device" ] && printf "  device=" && readlink -f "$l/device" 2>&1; done'

if [ "$SAFETY_VALIDATE" = "1" ]; then
    if [ -n "$LOAD_CMD" ]; then
        trap 'stop_load; restore_controls' EXIT INT TERM
        start_load || true
    fi
    run_safety_validation
    stop_load
    restore_controls
    post_restore_cooldown
    collect_final_logs
    write_summary
    trap - EXIT INT TERM
    print_output_path
    exit 0
fi

if [ "$FAN_MODE_SCAN" = "1" ]; then
    run_fan_mode_scan
    restore_controls
    post_restore_cooldown
    collect_final_logs
    write_summary
    trap - EXIT INT TERM
    print_output_path
    exit 0
fi

if [ "$CALIBRATE_FANS" = "1" ]; then
    run_fan_calibration
    restore_controls
    post_restore_cooldown
    collect_final_logs
    write_summary
    trap - EXIT INT TERM
    print_output_path
    exit 0
fi

if [ "$DAILY_MODE" = "1" ]; then
    sample_status daily-observation
    collect_final_logs
    write_summary
    trap - EXIT INT TERM
    print_output_path
    exit 0
fi

sample_status idle-baseline
run_phase idle-coolboost-off off
run_phase idle-coolboost-on on

if [ -n "$LOAD_CMD" ]; then
    trap 'stop_load; restore_controls' EXIT INT TERM

    if start_load; then
        progress "setting CoolBoost off for load-baseline"
        "$ANS" coolboost off > "$OUT_DIR/load-baseline.coolboost-set.txt" 2>&1 || true
        sample_status load-baseline
        run_phase load-coolboost-off off
        run_phase load-coolboost-on on
        if [ "$MODE_MATRIX" = "1" ]; then
            run_mode_phase load-firmware-auto off auto balanced
            run_mode_phase load-coolboost-turbo on auto balanced
            run_mode_phase load-fan-turbo off turbo balanced
            run_mode_phase load-performance-auto off auto performance
            run_mode_phase load-performance-coolboost on auto performance
            run_mode_phase load-performance-turbo off turbo performance
        fi
    fi

    stop_load
fi

restore_controls
if [ -n "$LOAD_CMD" ] && [ "$COOLDOWN_SECONDS" -gt 0 ]; then
    progress "cooling down for ${COOLDOWN_SECONDS}s"
    sleep "$COOLDOWN_SECONDS"
    sample_status post-load-cooldown
fi
post_restore_cooldown

collect_final_logs
write_summary
trap - EXIT INT TERM

print_output_path
