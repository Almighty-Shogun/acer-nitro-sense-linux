#!/usr/bin/env sh

run_fan_mode_scan() {
    file="$OUT_DIR/fan-mode-scan.status.log"

    progress "starting fan mode scan: profiles=$FAN_MODE_SCAN_PROFILES modes=$FAN_MODE_SCAN_MODES settle=${FAN_MODE_SCAN_SETTLE_SECONDS}s"
    "$ANS" coolboost off > "$OUT_DIR/fan-mode-scan.coolboost-set.txt" 2>&1 || true
    ec_snapshot "fan-mode-scan-start"

    {
        printf '# label=fan-mode-scan profiles="%s" modes="%s" settle=%ss\n' \
            "$FAN_MODE_SCAN_PROFILES" "$FAN_MODE_SCAN_MODES" "$FAN_MODE_SCAN_SETTLE_SECONDS"
        for profile in $FAN_MODE_SCAN_PROFILES; do
            case "$profile" in
                quiet|balanced|performance)
                    ;;
                *)
                    printf '\n## profile=%s skipped=invalid\n' "$profile"
                    continue
                    ;;
            esac

            progress "mode scan setting profile $profile"
            printf '\n## profile=%s command=profile timestamp=%s\n' "$profile" "$(date --iso-8601=seconds 2>/dev/null || date)"
            "$ANS" profile "$profile" 2>&1 || true
            ec_snapshot "fan-mode-scan-profile-${profile}"

            for mode in $FAN_MODE_SCAN_MODES; do
                case "$mode" in
                    auto|manual|turbo)
                        ;;
                    *)
                        printf '\n## profile=%s mode=%s skipped=invalid\n' "$profile" "$mode"
                        continue
                        ;;
                esac

                progress "mode scan profile $profile fan-mode $mode"
                printf '\n## profile=%s mode=%s command=fan-mode timestamp=%s\n' "$profile" "$mode" "$(date --iso-8601=seconds 2>/dev/null || date)"
                "$ANS" fan-mode "$mode" 2>&1 || true
                sleep "$FAN_MODE_SCAN_SETTLE_SECONDS"
                "$ANS" status 2>&1 || true
                sample_status_control_json || true
                "$ANS" coolboost status 2>&1 || true
                "$ANS" fan-mode status 2>&1 || true
                "$ANS" profile status 2>&1 || true
                ec_snapshot "fan-mode-scan-${profile}-${mode}"
            done
        done
    } > "$file"

    ec_snapshot "fan-mode-scan-end"
}
