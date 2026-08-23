#!/usr/bin/env sh

run_fan_calibration() {
    file="$OUT_DIR/fan-calibration.status.log"

    progress "starting fan calibration: targets=$CALIBRATION_TARGETS speeds=$CALIBRATION_SPEEDS fan-mode=$CALIBRATION_FAN_MODE profile=$CALIBRATION_PROFILE settle=${CALIBRATION_SETTLE_SECONDS}s samples=$CALIBRATION_SAMPLES"
    "$ANS" coolboost off > "$OUT_DIR/fan-calibration.coolboost-set.txt" 2>&1 || true
    case "$CALIBRATION_PROFILE" in
        quiet|balanced|performance)
            "$ANS" profile "$CALIBRATION_PROFILE" > "$OUT_DIR/fan-calibration.profile-set.txt" 2>&1 || true
            ;;
        *)
            progress "invalid calibration profile $CALIBRATION_PROFILE; keeping current profile"
            ;;
    esac
    case "$CALIBRATION_FAN_MODE" in
        auto|manual|turbo)
            "$ANS" fan-mode "$CALIBRATION_FAN_MODE" > "$OUT_DIR/fan-calibration.fan-mode-set.txt" 2>&1 || true
            ;;
        *)
            progress "invalid calibration fan mode $CALIBRATION_FAN_MODE; using manual"
            CALIBRATION_FAN_MODE="manual"
            "$ANS" fan-mode manual > "$OUT_DIR/fan-calibration.fan-mode-set.txt" 2>&1 || true
            ;;
    esac
    ec_snapshot "fan-calibration-start"

    {
        printf '# label=fan-calibration targets="%s" speeds="%s" fan_mode=%s profile=%s settle=%ss samples_per_speed=%s\n' \
            "$CALIBRATION_TARGETS" "$CALIBRATION_SPEEDS" "$CALIBRATION_FAN_MODE" \
            "$CALIBRATION_PROFILE" "$CALIBRATION_SETTLE_SECONDS" "$CALIBRATION_SAMPLES"
        for target in $CALIBRATION_TARGETS; do
            case "$target" in
                all|cpu|gpu)
                    ;;
                *)
                    printf '\n## target=%s skipped=invalid\n' "$target"
                    continue
                    ;;
            esac

            for speed in $CALIBRATION_SPEEDS; do
                case "$speed" in
                    ''|*[!0-9]*)
                        printf '\n## target=%s speed=%s skipped=invalid\n' "$target" "$speed"
                        continue
                        ;;
                esac

                if [ "$speed" -lt 1 ] || [ "$speed" -gt 100 ]; then
                    printf '\n## target=%s speed=%s skipped=out-of-range\n' "$target" "$speed"
                    continue
                fi

                progress "calibration target $target setting ${speed}%"
                printf '\n## target=%s speed=%s command=set-%s timestamp=%s\n' "$target" "$speed" "$target" "$(date --iso-8601=seconds 2>/dev/null || date)"
                case "$target" in
                    all)
                        "$ANS" set all "$speed" 2>&1 || true
                        ;;
                    cpu)
                        case "${INITIAL_GPU_REQUESTED:-}" in
                            ''|*[!0-9]*) "$ANS" set gpu 30 >/dev/null 2>&1 || true ;;
                            *) "$ANS" set gpu "$INITIAL_GPU_REQUESTED" >/dev/null 2>&1 || true ;;
                        esac
                        "$ANS" set cpu "$speed" 2>&1 || true
                        ;;
                    gpu)
                        case "${INITIAL_CPU_REQUESTED:-}" in
                            ''|*[!0-9]*) "$ANS" set cpu 40 >/dev/null 2>&1 || true ;;
                            *) "$ANS" set cpu "$INITIAL_CPU_REQUESTED" >/dev/null 2>&1 || true ;;
                        esac
                        "$ANS" set gpu "$speed" 2>&1 || true
                        ;;
                esac
                case "$CALIBRATION_FAN_MODE" in
                    auto|turbo)
                        "$ANS" fan-mode "$CALIBRATION_FAN_MODE" >/dev/null 2>&1 || true
                        ;;
                esac
                ec_snapshot "fan-calibration-${target}-${speed}-after-command"
                sleep "$CALIBRATION_SETTLE_SECONDS"
                ec_snapshot "fan-calibration-${target}-${speed}-after-settle"

                j=1
                while [ "$j" -le "$CALIBRATION_SAMPLES" ]; do
                    progress "calibration target $target speed ${speed}% sample $j/$CALIBRATION_SAMPLES"
                    printf '\n## target=%s speed=%s sample=%s timestamp=%s\n' "$target" "$speed" "$j" "$(date --iso-8601=seconds 2>/dev/null || date)"
                    "$ANS" status 2>&1 || true
                    sample_status_control_json || true
                    "$ANS" fan-mode status 2>&1 || true
                    "$ANS" profile status 2>&1 || true
                    j=$((j + 1))
                    if [ "$j" -le "$CALIBRATION_SAMPLES" ]; then
                        sleep "$INTERVAL"
                    fi
                done
            done
        done
    } > "$file"
    ec_snapshot "fan-calibration-end"
}
