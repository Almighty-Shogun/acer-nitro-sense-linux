write_summary_header() {
    printf 'Acer Nitro Sense validation summary\n\n'
    printf 'Output directory: %s\n' "$OUT_DIR"
    printf 'Initial CoolBoost: %s\n' "${INITIAL_COOLBOOST:-unknown}"
    printf 'Initial fan mode: %s\n' "${INITIAL_FAN_MODE:-unknown}"
    printf 'Initial profile: %s\n' "${INITIAL_PROFILE:-unknown}"
    printf 'Initial power source: %s\n' "${INITIAL_POWER_SOURCE:-unknown}"
    printf 'Initial GPU temp policy: %s\n' "${INITIAL_GPU_TEMP_POLICY:-unknown}"
    printf 'Initial control mode: %s\n' "${INITIAL_CONTROL_MODE:-unknown}"
    printf 'Initial preset: %s\n' "${INITIAL_PRESET:-unknown}"
    printf 'Initial CPU requested: %s\n' "${INITIAL_CPU_REQUESTED:-unknown}"
    printf 'Initial GPU requested: %s\n' "${INITIAL_GPU_REQUESTED:-unknown}"
    printf 'Samples: %s\n' "$SAMPLES"
    printf 'Interval: %ss\n' "$INTERVAL"
    printf 'Load command: %s\n\n' "${LOAD_CMD:-none}"
    printf 'Cooldown: %ss\n\n' "$COOLDOWN_SECONDS"
    printf 'Post-restore cooldown: %ss\n\n' "$RESTORE_COOLDOWN_SECONDS"
    printf 'Mode matrix: %s\n\n' "$MODE_MATRIX"
    printf 'Daily mode: %s\n\n' "$DAILY_MODE"
    printf 'Fan calibration: %s\n' "$CALIBRATE_FANS"
    printf 'Calibration targets: %s\n' "$CALIBRATION_TARGETS"
    printf 'Calibration speeds: %s\n' "$CALIBRATION_SPEEDS"
    printf 'Calibration fan mode: %s\n' "$CALIBRATION_FAN_MODE"
    printf 'Calibration profile: %s\n' "$CALIBRATION_PROFILE"
    printf 'Calibration settle: %ss\n' "$CALIBRATION_SETTLE_SECONDS"
    printf 'Calibration samples per speed: %s\n\n' "$CALIBRATION_SAMPLES"
    printf 'EC dump: %s range=%s-%s\n' "$EC_DUMP" "$EC_DUMP_START" "$EC_DUMP_END"
    printf 'Fan mode scan: %s\n' "$FAN_MODE_SCAN"
    printf 'Fan mode scan modes: %s\n' "$FAN_MODE_SCAN_MODES"
    printf 'Fan mode scan profiles: %s\n\n' "$FAN_MODE_SCAN_PROFILES"
    printf 'Safety validation: %s\n' "$SAFETY_VALIDATE"
    printf 'Safety validation modes: %s\n' "$SAFETY_VALIDATE_MODES"
    printf 'Safety validation percent: %s\n' "$SAFETY_VALIDATE_PERCENT"
    printf 'Safety validation preset: %s\n' "$SAFETY_VALIDATE_PRESET"
    printf 'Safety validation settle: %ss\n\n' "$SAFETY_VALIDATE_SETTLE_SECONDS"
}

write_final_section() {
    title="$1"
    file="$2"

    printf '\n%s:\n' "$title"
    if [ -f "$file" ]; then
        awk 'substr($0, 1, 2) != "+ " {print; shown++} shown == 20 {exit}' "$file"
    fi
}

write_final_summaries() {
    write_final_section "Final status" "$OUT_DIR/final-status.txt"
    write_final_section "Final capabilities" "$OUT_DIR/final-capabilities.txt"
    write_final_section "Final CoolBoost" "$OUT_DIR/final-coolboost.txt"
    write_final_section "Final fan mode" "$OUT_DIR/final-fan-mode.txt"
    write_final_section "Final profile" "$OUT_DIR/final-profile.txt"
    write_final_section "Final power source" "$OUT_DIR/final-power-source.txt"
    write_final_section "Final GPU temp policy" "$OUT_DIR/final-gpu-temp.txt"
    write_final_section "Final keyboard backlight" "$OUT_DIR/final-keyboard-backlight.txt"
}

write_summary() {
    summary="$OUT_DIR/summary.txt"

    {
        write_summary_header
        write_feature_summary

        if [ -f "$OUT_DIR/load.failed.txt" ]; then
            printf 'Load failure:\n'
            sed -n '1,40p' "$OUT_DIR/load.failed.txt"
            printf '\n'
        fi

        write_status_log_summaries
        write_fan_calibration_summary
        write_final_summaries
    } > "$summary"

    progress "summary written to $summary"
}
