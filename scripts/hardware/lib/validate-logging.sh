#!/usr/bin/env sh

progress() {
    printf '[%s] %s\n' "$(date +%H:%M:%S)" "$*" >&2
}

print_output_path() {
    printf '\n---\n%s\n' "$OUT_DIR"
}

run_log() {
    label="$1"
    shift
    progress "collecting $label"
    {
        printf '+ %s\n' "$*"
        "$@" 2>&1 || true
    } > "$OUT_DIR/$label.txt"
}

collect_current_daemon_journal() {
    label="$1"
    since="${VALIDATION_STARTED_AT:-}"

    if [ -z "$since" ]; then
        since="$(systemctl show -P ActiveEnterTimestamp acer-nitro-sense.service 2>/dev/null || true)"
    fi

    run_log "$label" env ANS_JOURNAL_SINCE="$since" sh -c "if [ -n \"\$ANS_JOURNAL_SINCE\" ]; then journalctl -u acer-nitro-sense.service --since \"\$ANS_JOURNAL_SINCE\" --no-pager 2>&1; else journalctl -u acer-nitro-sense.service -n 120 --no-pager 2>&1; fi"
}

collect_final_logs() {
    run_log final-status "$ANS" status
    run_log final-capabilities "$ANS" capabilities
    run_log final-coolboost "$ANS" coolboost status
    run_log final-fan-mode "$ANS" fan-mode status
    run_log final-profile "$ANS" profile status
    run_log final-power-source "$ANS" power-source status
    run_log final-gpu-temp "$ANS" gpu-temp status
    run_log final-keyboard-backlight "$ANS" keyboard-backlight status
    collect_current_daemon_journal final-journal-current-daemon
}

ec_snapshot() {
    label="$1"

    if [ "$EC_DUMP" != "1" ]; then
        return
    fi

    {
        printf '\n## label=%s timestamp=%s\n' "$label" "$(date --iso-8601=seconds 2>/dev/null || date)"
        "$ANS" status 2>&1 || true
        "$ANS" fan-mode status 2>&1 || true
        "$ANS" profile status 2>&1 || true
        "$ANS" ec dump "$EC_DUMP_START" "$EC_DUMP_END" 2>&1 || true
    } >> "$OUT_DIR/ec-dump.log"
}

sample_status_control_json() {
    "$ANS" status --json 2>/dev/null | awk '
function json_num(line, key,    needle, rest) {
    needle = "\"" key "\": "
    if (index(line, needle) == 0)
        return -1
    rest = substr(line, index(line, needle) + length(needle))
    sub(/[, ].*/, "", rest)
    return rest + 0
}
/"id": "cpu"/ || /"id": "gpu"/ {
    fan = ""
    if (index($0, "\"id\": \"cpu\"") > 0)
        fan = "cpu"
    else if (index($0, "\"id\": \"gpu\"") > 0)
        fan = "gpu"

    if (fan != "")
        printf "control fan=%s temp=%d sensor_temp=%d control_temp=%d control_sensor_temp=%d critical_samples=%d\n",
            fan,
            json_num($0, "temp_c"),
            json_num($0, "sensor_temp_c"),
            json_num($0, "control_temp_c"),
            json_num($0, "control_sensor_temp_c"),
            json_num($0, "critical_temp_samples")
}'
}

sample_status() {
    label="$1"
    file="$OUT_DIR/$label.status.log"
    i=1

    progress "sampling $label ($SAMPLES samples, ${INTERVAL}s interval)"
    {
        printf '# label=%s samples=%s interval=%ss\n' "$label" "$SAMPLES" "$INTERVAL"
        while [ "$i" -le "$SAMPLES" ]; do
            progress "$label sample $i/$SAMPLES"
            printf '\n## sample=%s timestamp=%s\n' "$i" "$(date --iso-8601=seconds 2>/dev/null || date)"
            "$ANS" status 2>&1 || true
            sample_status_control_json || true
            "$ANS" coolboost status 2>&1 || true
            "$ANS" fan-mode status 2>&1 || true
            "$ANS" profile status 2>&1 || true
            "$ANS" power-source status 2>&1 || true
            i=$((i + 1))
            if [ "$i" -le "$SAMPLES" ]; then
                sleep "$INTERVAL"
            fi
        done
    } > "$file"
}
