run_safety_validation() {
    file="$OUT_DIR/safety-validation.status.log"

    progress "starting safety validation: modes=$SAFETY_VALIDATE_MODES percent=${SAFETY_VALIDATE_PERCENT}% preset=$SAFETY_VALIDATE_PRESET settle=${SAFETY_VALIDATE_SETTLE_SECONDS}s"
    "$ANS" coolboost off > "$OUT_DIR/safety-validation.coolboost-set.txt" 2>&1 || true
    ec_snapshot "safety-validation-start"

    {
        printf '# label=safety-validation modes="%s" percent=%s preset=%s settle=%ss\n' \
            "$SAFETY_VALIDATE_MODES" "$SAFETY_VALIDATE_PERCENT" \
            "$SAFETY_VALIDATE_PRESET" "$SAFETY_VALIDATE_SETTLE_SECONDS"
        for mode in $SAFETY_VALIDATE_MODES; do
            case "$mode" in
                manual)
                    progress "safety validation mode manual at ${SAFETY_VALIDATE_PERCENT}%"
                    printf '\n## mode=manual command=set-all timestamp=%s\n' "$(date --iso-8601=seconds 2>/dev/null || date)"
                    "$ANS" fan-mode manual 2>&1 || true
                    "$ANS" set all "$SAFETY_VALIDATE_PERCENT" 2>&1 || true
                    ;;
                preset)
                    progress "safety validation preset $SAFETY_VALIDATE_PRESET"
                    printf '\n## mode=preset preset=%s command=preset timestamp=%s\n' "$SAFETY_VALIDATE_PRESET" "$(date --iso-8601=seconds 2>/dev/null || date)"
                    "$ANS" preset "$SAFETY_VALIDATE_PRESET" 2>&1 || true
                    ;;
                auto)
                    progress "safety validation daemon auto"
                    printf '\n## mode=auto command=auto timestamp=%s\n' "$(date --iso-8601=seconds 2>/dev/null || date)"
                    "$ANS" auto 2>&1 || true
                    ;;
                firmware-auto)
                    progress "safety validation firmware auto"
                    printf '\n## mode=firmware-auto command=firmware-auto timestamp=%s\n' "$(date --iso-8601=seconds 2>/dev/null || date)"
                    "$ANS" firmware-auto 2>&1 || true
                    ;;
                *)
                    printf '\n## mode=%s skipped=invalid\n' "$mode"
                    continue
                    ;;
            esac

            sleep "$SAFETY_VALIDATE_SETTLE_SECONDS"
            "$ANS" status 2>&1 || true
            sample_status_control_json || true
            "$ANS" coolboost status 2>&1 || true
            "$ANS" fan-mode status 2>&1 || true
            "$ANS" profile status 2>&1 || true
            ec_snapshot "safety-validation-${mode}"
        done
    } > "$file"

    ec_snapshot "safety-validation-end"
}
