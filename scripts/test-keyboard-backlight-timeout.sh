#!/usr/bin/env sh
set -u

ANS="${ANS:-ans}"
OUT_DIR="${OUT_DIR:-/tmp/acer-nitro-sense-keyboard-backlight-timeout-$(date +%Y%m%d-%H%M%S)}"
LOG_FILE="$OUT_DIR/keyboard-backlight-timeout.log"
SETTLE_SECONDS="${SETTLE_SECONDS:-3}"
EXTRA_WAIT_SECONDS="${EXTRA_WAIT_SECONDS:-8}"

mkdir -p "$OUT_DIR"

log() {
    printf '%s\n' "$*" | tee -a "$LOG_FILE"
}

run_cmd() {
    tmp="$OUT_DIR/command-output.tmp"

    log "$ $*"
    "$@" >"$tmp" 2>&1
    status=$?
    cat "$tmp" | tee -a "$LOG_FILE"
    rm -f "$tmp"
    log "exit=$status"
    return "$status"
}

status_line() {
    "$ANS" keyboard-backlight status 2>/dev/null | sed -n '1p'
}

field_value() {
    key="$1"
    line="$2"
    printf '%s\n' "$line" | tr ' ' '\n' | sed -n "s/^${key}=//p" | sed -n '1p'
}

ask() {
    prompt="$1"
    printf '%s ' "$prompt" | tee -a "$LOG_FILE"
    IFS= read -r answer
    printf '%s\n' "$answer" >> "$LOG_FILE"
    case "$answer" in
        y|Y|yes|YES) log "observed=yes" ;;
        *) log "observed=no" ;;
    esac
}

initial_line="$(status_line)"
initial_percent="$(field_value percent "$initial_line")"
initial_timeout="$(field_value timeout "$initial_line")"
timeout_seconds="$(field_value timeout_seconds "$initial_line")"

case "$initial_percent" in
    ''|*[!0-9]*) initial_percent=100 ;;
esac
case "$timeout_seconds" in
    ''|*[!0-9]*) timeout_seconds=30 ;;
esac

wait_seconds=$((timeout_seconds + EXTRA_WAIT_SECONDS))

{
    printf 'Acer Nitro Sense keyboard backlight timeout test\n\n'
    printf 'Command: %s\n' "$ANS"
    printf 'Initial status: %s\n' "$initial_line"
    printf 'Timeout seconds: %s\n' "$timeout_seconds"
    printf 'Hands-off wait seconds: %s\n\n' "$wait_seconds"
} | tee "$OUT_DIR/summary.txt" | tee -a "$LOG_FILE" >/dev/null

run_cmd "$ANS" keyboard-backlight set 100
run_cmd "$ANS" keyboard-backlight timeout on
run_cmd "$ANS" keyboard-backlight status

log ""
log "Do not touch the keyboard, touchpad, or mouse for ${wait_seconds}s."
log "Watch whether the keyboard backlight turns off around ${timeout_seconds}s."
sleep "$wait_seconds"

run_cmd "$ANS" keyboard-backlight status
ask "Did the keyboard backlight turn off after the timeout? [y/N]"

log ""
printf 'Press Enter once to generate activity and continue...' | tee -a "$LOG_FILE"
IFS= read -r _
printf '\n' >> "$LOG_FILE"
sleep "$SETTLE_SECONDS"

run_cmd "$ANS" keyboard-backlight status
ask "Did the keyboard backlight restore after activity? [y/N]"

log ""
log "Restoring initial keyboard backlight settings."
run_cmd "$ANS" keyboard-backlight set "$initial_percent"
case "$initial_timeout" in
    on|off) run_cmd "$ANS" keyboard-backlight timeout "$initial_timeout" ;;
esac
run_cmd "$ANS" keyboard-backlight status

{
    printf '\nResult summary:\n'
    grep -E '^\$ |^keyboard_backlight=|^keyboard_backlight_timeout=|^observed=' "$LOG_FILE" || true
    printf '\n---\n%s\n' "$OUT_DIR"
} | tee "$OUT_DIR/summary.txt"

printf '\n---\n%s\n' "$OUT_DIR"
