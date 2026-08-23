#!/usr/bin/env sh
set -eu

ANS="${ANS:-ans}"
STEPS="${STEPS:-4}"
ACTION_SEQUENCE="${ACTION_SEQUENCE:-F10 F10 F10 F9}"
SETTLE_SECONDS="${SETTLE_SECONDS:-3}"
EVENT_SECONDS="${EVENT_SECONDS:-6}"
EC_DUMP_RANGES="${EC_DUMP_RANGES:-0x00-0x7f 0x80-0xff}"
OUT_DIR="${OUT_DIR:-/tmp/acer-nitro-sense-keyboard-backlight-$(date +%Y%m%d-%H%M%S)}"
LOG_FILE="$OUT_DIR/keyboard-backlight-discovery.log"
STARTED_AT="$(date --iso-8601=seconds 2>/dev/null || date)"
SCRIPT_DIR="$(CDPATH='' cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(CDPATH='' cd "$SCRIPT_DIR/../.." && pwd)"
export LOG_FILE

case "$STEPS" in
    ''|*[!0-9]*)
        printf 'STEPS must be a non-negative integer\n' >&2
        exit 2
        ;;
esac

case "$SETTLE_SECONDS" in
    ''|*[!0-9]*)
        printf 'SETTLE_SECONDS must be a non-negative integer\n' >&2
        exit 2
        ;;
esac

case "$EVENT_SECONDS" in
    ''|*[!0-9]*)
        printf 'EVENT_SECONDS must be a non-negative integer\n' >&2
        exit 2
        ;;
esac

mkdir -p "$OUT_DIR"
cd "$PROJECT_DIR"

# shellcheck source=scripts/hardware/lib/keyboard-backlight-discovery.sh
. "$SCRIPT_DIR/lib/keyboard-backlight-discovery.sh"

write_ec_diff() {
    from="$1"
    to="$2"
    diff_file="$OUT_DIR/$to-ec-diff-from-$from.txt"

    {
        printf '## EC diff %s -> %s\n' "$from" "$to"
        diff -u "$OUT_DIR/$from-ec-dump.txt" "$OUT_DIR/$to-ec-dump.txt" 2>&1 || true
    } > "$diff_file"
}

write_ec_register_matrix() {
    matrix_file="$OUT_DIR/ec-register-matrix.txt"

    {
        for dump_file in "$OUT_DIR"/initial-ec-dump.txt "$OUT_DIR"/step-*-ec-dump.txt; do
            [ -f "$dump_file" ] || continue
            label=$(basename "$dump_file" -ec-dump.txt)
            awk -v label="$label" '/^0x[0-9a-f][0-9a-f]:/ {
                base=substr($1, 3, 2)
                sub(":", "", base)
                for (i = 2; i <= NF; i++)
                    printf "%s 0x%02x %s\n", label, strtonum("0x" base) + i - 2, $i
            }' "$dump_file"
        done
    } | awk '
        {
            v[$2,$1] = $3
            regs[$2] = 1
            labels[$1] = 1
        }
        END {
            label_count = 0
            for (label in labels)
                ordered[++label_count] = label
            for (i = 1; i <= label_count; i++) {
                for (j = i + 1; j <= label_count; j++) {
                    if (ordered[j] < ordered[i]) {
                        tmp = ordered[i]
                        ordered[i] = ordered[j]
                        ordered[j] = tmp
                    }
                }
            }
            for (r in regs) {
                first = v[r,"initial"]
                changed = 0
                for (i = 1; i <= label_count; i++) {
                    label = ordered[i]
                    if (label != "initial" && v[r,label] != first)
                        changed = 1
                }
                if (changed) {
                    printf "%s", r
                    for (i = 1; i <= label_count; i++) {
                        label = ordered[i]
                        printf " %s=%s", label, v[r,label]
                    }
                    printf "\n"
                }
            }
        }
    ' | sort > "$matrix_file"
}

write_summary() {
    summary_file="$OUT_DIR/summary.txt"

    write_ec_register_matrix

    {
        printf 'Acer Nitro Sense keyboard backlight discovery\n\n'
        printf 'Output directory: %s\n' "$OUT_DIR"
        printf 'Started at: %s\n' "$STARTED_AT"
        printf 'ANS: %s\n' "$ANS"
        printf 'Steps: %s\n' "$STEPS"
        printf 'Action sequence: %s\n' "$ACTION_SEQUENCE"
        printf 'Settle seconds: %s\n' "$SETTLE_SECONDS"
        printf 'Event capture seconds: %s\n' "$EVENT_SECONDS"
        printf 'EC ranges: %s\n\n' "$EC_DUMP_RANGES"

        printf 'Initial daemon keyboard-backlight status:\n'
        sed -n '/^keyboard_backlight=/p' "$OUT_DIR/initial-keyboard-backlight-status.txt" 2>/dev/null || true

        printf '\nSnapshot files:\n'
        find "$OUT_DIR" -maxdepth 1 -type f \
            \( -name '*keyboard-backlight-status.txt' -o -name '*leds.txt' -o -name '*wmi.txt' -o -name '*input.txt' -o -name '*live-input-events.txt' -o -name '*ec-dump.txt' -o -name '*ec-diff-from-*.txt' \) \
            -printf '  %f\n' 2>/dev/null | sort

        printf '\nLive input event highlights:\n'
        found_events=0
        for event_file in "$OUT_DIR"/*-live-input-events.txt; do
            [ -e "$event_file" ] || continue
            if grep -Eiq 'kbdillum|brightness|KEY_|MSC_SCAN|acer_wmi|permission denied' "$event_file"; then
                found_events=1
                printf '\n%s\n' "$(basename "$event_file")"
                grep -Ei 'kbdillum|brightness|KEY_|MSC_SCAN|acer_wmi|permission denied' "$event_file" || true
            fi
        done
        if [ "$found_events" -eq 0 ]; then
            printf '  none\n'
        fi

        printf '\nChanged EC register matrix:\n'
        sed -n '1,80p' "$OUT_DIR/ec-register-matrix.txt" 2>/dev/null || true

        printf '\nLikely keyboard backlight candidates:\n'
        if grep -q '^0x31 ' "$OUT_DIR/ec-register-matrix.txt" 2>/dev/null; then
            grep '^0x31 ' "$OUT_DIR/ec-register-matrix.txt"
        else
            printf '  none yet\n'
        fi

        printf '\nEC rows that changed compared with initial:\n'
        found=0
        for diff_file in "$OUT_DIR"/*-ec-diff-from-initial.txt; do
            [ -e "$diff_file" ] || continue
            if grep -q '^[+-]0x' "$diff_file"; then
                found=1
                printf '\n%s\n' "$(basename "$diff_file")"
                grep '^[+-]0x' "$diff_file" || true
            fi
        done
        if [ "$found" -eq 0 ]; then
            printf '  none\n'
        fi

        printf '\nKernel messages since start:\n'
        sed -n '1,120p' "$OUT_DIR/kernel-journal-since-start.txt" 2>/dev/null || true

        printf '\n---\n%s\n' "$OUT_DIR"
    } > "$summary_file"
}

log "writing keyboard backlight discovery logs to $OUT_DIR"

cat > "$OUT_DIR/README.txt" <<EOF
Acer Nitro Sense keyboard backlight discovery

This script captures daemon, LED sysfs, backlight sysfs, live input events,
input/WMI device state, and EC state around keyboard-backlight hotkey actions.

For live input events, run it as root or as a user that can read /dev/input.
At each step, press Enter to start the event window, press the prompted hotkey
once during that window, and then wait for the EC/sysfs snapshot.
EOF

capture_state "initial"

i=1
while [ "$i" -le "$STEPS" ]; do
    action="$(action_for_step "$i")"
    action_text="$(display_action "$action")"
    prompt_capture "Step $i/$STEPS: press $action_text once during the capture window."
    capture_live_events "step-$i"
    wait_settle "keyboard backlight step $i"
    capture_state "step-$i"
    write_ec_diff "initial" "step-$i"
    i=$((i + 1))
done

run_cmd "kernel-journal-since-start" env ANS_JOURNAL_SINCE="$STARTED_AT" sh -c "
    journalctl -k --since \"\$ANS_JOURNAL_SINCE\" --no-pager 2>&1 || true
"

write_summary
log "keyboard backlight discovery complete"

printf '\n---\n%s\n' "$OUT_DIR"
