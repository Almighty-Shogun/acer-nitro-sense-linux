#!/usr/bin/env sh
set -u

ANS="${ANS:-ans}"
OUT_DIR="${OUT_DIR:-/tmp/acer-nitro-sense-keyboard-backlight-control-$(date +%Y%m%d-%H%M%S)}"
LOG_FILE="$OUT_DIR/keyboard-backlight-control.log"
LEVELS="${LEVELS:-0 25 50 75 100}"
SETTLE_SECONDS="${SETTLE_SECONDS:-2}"

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

{
    printf 'Acer Nitro Sense keyboard backlight control test\n\n'
    printf 'Command: %s\n' "$ANS"
    printf 'Levels: %s\n' "$LEVELS"
    printf 'Settle seconds: %s\n\n' "$SETTLE_SECONDS"
} | tee "$OUT_DIR/summary.txt" | tee -a "$LOG_FILE" >/dev/null

run_cmd "$ANS" keyboard-backlight status

for level in $LEVELS; do
    log ""
    log "Set keyboard backlight to ${level}%."
    printf 'Press Enter to write %s%%...' "$level" | tee -a "$LOG_FILE"
    IFS= read -r _
    printf '\n' >> "$LOG_FILE"

    run_cmd "$ANS" keyboard-backlight set "$level"
    sleep "$SETTLE_SECONDS"
    run_cmd "$ANS" keyboard-backlight status
    ask "Did the keyboard backlight visibly match ${level}%? [y/N]"
done

{
    printf '\nResult summary:\n'
    grep -E '^\$ |^keyboard_backlight=|^observed=' "$LOG_FILE" || true
    printf '\n---\n%s\n' "$OUT_DIR"
} | tee "$OUT_DIR/summary.txt"

printf '\n---\n%s\n' "$OUT_DIR"
