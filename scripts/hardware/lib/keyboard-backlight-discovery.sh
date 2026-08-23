#!/usr/bin/env sh

timestamp() {
    date --iso-8601=seconds 2>/dev/null || date
}

log() {
    printf '[%s] %s\n' "$(timestamp)" "$*" | tee -a "$LOG_FILE"
}

command_file_name() {
    printf '%s' "$1" | tr ' /' '--' | tr -cd 'A-Za-z0-9._-'
}

run_cmd() {
    run_label="$1"
    shift
    run_file="$OUT_DIR/$(command_file_name "$run_label").txt"
    status=0

    log "collecting: $run_label"
    {
        printf '## %s\n' "$run_label"
        printf 'timestamp=%s\n' "$(timestamp)"
        printf '$'
        for arg in "$@"; do
            printf ' %s' "$arg"
        done
        printf '\n\n'
        set +e
        "$@" 2>&1
        status=$?
        set -e
        printf '\nexit_code=%s\n' "$status"
    } > "$run_file"

    cat "$run_file" >> "$LOG_FILE"
    printf '\n' >> "$LOG_FILE"
}

prompt_capture() {
    message="$1"

    log "$message"
    printf '%s\nPress Enter to start the %ss event capture window...' \
        "$message" "$EVENT_SECONDS"
    read -r _answer
    log "starting event capture"
}

action_for_step() {
    action_step="$1"

    printf '%s\n' "$ACTION_SEQUENCE" | tr ' ' '\n' | sed -n "${action_step}p"
}

display_action() {
    action="$1"

    case "$action" in
        F[0-9]|F1[0-2])
            printf 'Fn + %s' "$action"
            ;;
        '')
            printf 'the keyboard backlight hotkey'
            ;;
        *)
            printf '%s' "$action"
            ;;
    esac
}

wait_settle() {
    reason="$1"

    if [ "$SETTLE_SECONDS" -gt 0 ]; then
        log "waiting ${SETTLE_SECONDS}s for $reason"
        sleep "$SETTLE_SECONDS"
    fi
}

capture_leds() {
    label="$1"

    run_cmd "$label leds" sh -c "
        printf '## /sys/class/leds\n'
        ls -la /sys/class/leds 2>&1 || true
        for led in /sys/class/leds/*; do
            [ -e \"\$led\" ] || continue
            printf '\n%s\n' \"\$led\"
            for file in brightness max_brightness trigger delay_on delay_off color function device_name; do
                [ -r \"\$led/\$file\" ] && printf '  %s=' \"\$file\" && cat \"\$led/\$file\" 2>&1
            done
            [ -e \"\$led/device\" ] && printf '  device=' && readlink -f \"\$led/device\" 2>&1
        done

        printf '\n## /sys/class/backlight\n'
        ls -la /sys/class/backlight 2>&1 || true
        for backlight in /sys/class/backlight/*; do
            [ -e \"\$backlight\" ] || continue
            printf '\n%s\n' \"\$backlight\"
            for file in brightness actual_brightness max_brightness bl_power scale type; do
                [ -r \"\$backlight/\$file\" ] && printf '  %s=' \"\$file\" && cat \"\$backlight/\$file\" 2>&1
            done
            [ -e \"\$backlight/device\" ] && printf '  device=' && readlink -f \"\$backlight/device\" 2>&1
        done
    "
}

capture_wmi() {
    label="$1"

    run_cmd "$label wmi" sh -c "
        printf '## modules\n'
        lsmod | grep -E '(^acer_wmi|^wmi|^wmi_bmof|^sparse_keymap)' || true

        printf '\n## WMI devices\n'
        ls -la /sys/bus/wmi/devices 2>&1 || true
        for dev in /sys/bus/wmi/devices/*; do
            [ -e \"\$dev\" ] || continue
            printf '\n%s\n' \"\$dev\"
            for file in modalias uevent guid object_id setable expensive notify_id; do
                [ -r \"\$dev/\$file\" ] && printf '  %s=' \"\$file\" && cat \"\$dev/\$file\" 2>&1
            done
            [ -e \"\$dev/driver\" ] && printf '  driver=' && basename \"\$(readlink -f \"\$dev/driver\")\" 2>&1
        done

        printf '\n## platform drivers\n'
        find /sys/bus/platform/drivers -maxdepth 1 \( -iname '*acer*' -o -iname '*wmi*' \) 2>&1 | sort
    "
}

capture_input() {
    label="$1"

    run_cmd "$label input" sh -c "
        printf '## /proc/bus/input/devices\n'
        cat /proc/bus/input/devices 2>&1 || true

        printf '\n## input event devices\n'
        ls -la /dev/input 2>&1 || true

        printf '\n## libinput\n'
        command -v libinput >/dev/null && libinput list-devices 2>&1 || echo libinput=unavailable
    "
}

capture_live_events() {
    label="$1"

    run_cmd "$label live-input-events" env EVENT_SECONDS="$EVENT_SECONDS" sh -c "
        printf '## live libinput events\n'
        if command -v libinput >/dev/null; then
            timeout \"\$EVENT_SECONDS\" libinput debug-events --show-keycodes 2>&1 || true
        else
            echo libinput=unavailable
        fi

        printf '\n## live acpi events\n'
        if command -v acpi_listen >/dev/null; then
            timeout \"\$EVENT_SECONDS\" acpi_listen 2>&1 || true
        else
            echo acpi_listen=unavailable
        fi
    "
}

capture_ec() {
    label="$1"

    run_cmd "$label ec-dump" env ANS_BIN="$ANS" EC_DUMP_RANGES="$EC_DUMP_RANGES" sh -c "
        for range in \$EC_DUMP_RANGES; do
            start=\"\${range%-*}\"
            end=\"\${range#*-}\"
            printf '\n## ec range %s-%s\n' \"\$start\" \"\$end\"
            \"\$ANS_BIN\" ec dump \"\$start\" \"\$end\" 2>&1
        done
    "
}

capture_state() {
    label="$1"

    run_cmd "$label keyboard-backlight-status" "$ANS" keyboard-backlight status
    run_cmd "$label capabilities" "$ANS" capabilities
    run_cmd "$label status" "$ANS" status
    capture_leds "$label"
    capture_wmi "$label"
    capture_input "$label"
    capture_ec "$label"
}
