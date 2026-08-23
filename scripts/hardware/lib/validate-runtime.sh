#!/usr/bin/env sh

coolboost_state() {
    "$ANS" coolboost status 2>/dev/null | sed -n 's/^coolboost=//p' | head -n 1
}

fan_mode_state() {
    "$ANS" fan-mode status 2>/dev/null | sed -n 's/^fan_mode=\([^ ]*\).*/\1/p' | head -n 1
}

profile_state() {
    "$ANS" profile status 2>/dev/null | sed -n 's/^profile=\([^ ]*\).*/\1/p' | head -n 1
}

power_source_state() {
    "$ANS" power-source status 2>/dev/null | sed -n 's/^power_source=\([^ ]*\).*/\1/p' | head -n 1
}

gpu_temp_policy_state() {
    "$ANS" gpu-temp status 2>/dev/null | sed -n 's/^gpu_temp=.* policy=\([^ ]*\).*/\1/p' | head -n 1
}

control_mode_state() {
    "$ANS" status 2>/dev/null | sed -n 's/^model=.* mode=\([^ ]*\).*/\1/p' | head -n 1
}

preset_state() {
    "$ANS" status 2>/dev/null | sed -n 's/^model=.* preset=\([^ ]*\).*/\1/p' | head -n 1
}

fan_requested_state() {
    fan="$1"
    "$ANS" status 2>/dev/null | awk -v fan="$fan" '
$1 == fan {
    for (i = 1; i <= NF; i++) {
        if ($i ~ /^requested=/) {
            sub(/^requested=/, "", $i)
            print $i
            exit
        }
    }
    for (i = 1; i <= NF; i++) {
        if ($i ~ /^percent=/) {
            sub(/^percent=/, "", $i)
            print $i
            exit
        }
    }
}'
}

restore_coolboost() {
    progress "turning CoolBoost off"
    "$ANS" coolboost off >/dev/null 2>&1 || true
}

restore_fan_mode() {
    case "${INITIAL_FAN_MODE:-}" in
        auto|manual)
            progress "restoring fan mode $INITIAL_FAN_MODE"
            "$ANS" fan-mode "$INITIAL_FAN_MODE" >/dev/null 2>&1 || true
            ;;
        *)
            progress "restoring fan mode manual"
            "$ANS" fan-mode manual >/dev/null 2>&1 || true
            ;;
    esac
}

restore_profile() {
    case "${INITIAL_PROFILE:-}" in
        quiet|balanced|performance)
            progress "restoring profile $INITIAL_PROFILE"
            "$ANS" profile "$INITIAL_PROFILE" >/dev/null 2>&1 || true
            ;;
    esac
}

restore_fan_speeds() {
    case "${INITIAL_CONTROL_MODE:-}" in
        manual|firmware-auto)
            ;;
        *)
            return
            ;;
    esac

    case "${INITIAL_CPU_REQUESTED:-}" in
        ''|*[!0-9]*)
            ;;
        *)
            progress "restoring CPU fan speed $INITIAL_CPU_REQUESTED"
            "$ANS" set cpu "$INITIAL_CPU_REQUESTED" >/dev/null 2>&1 || true
            ;;
    esac

    case "${INITIAL_GPU_REQUESTED:-}" in
        ''|*[!0-9]*)
            ;;
        *)
            progress "restoring GPU fan speed $INITIAL_GPU_REQUESTED"
            "$ANS" set gpu "$INITIAL_GPU_REQUESTED" >/dev/null 2>&1 || true
            ;;
    esac
}

restore_daemon_control() {
    case "${INITIAL_CONTROL_MODE:-}" in
        auto)
            progress "restoring daemon control mode auto"
            "$ANS" auto >/dev/null 2>&1 || true
            ;;
        firmware-auto)
            progress "restoring daemon control mode firmware-auto"
            "$ANS" firmware-auto >/dev/null 2>&1 || true
            ;;
        preset)
            case "${INITIAL_PRESET:-}" in
                ''|manual|auto)
                    ;;
                *)
                    progress "restoring daemon preset $INITIAL_PRESET"
                    "$ANS" preset "$INITIAL_PRESET" >/dev/null 2>&1 || true
                    ;;
            esac
            ;;
    esac
}

restore_controls() {
    restore_coolboost
    restore_profile
    restore_fan_mode
    restore_fan_speeds
    restore_daemon_control
}

post_restore_cooldown() {
    if [ "${RESTORE_COOLDOWN_SECONDS:-0}" -le 0 ]; then
        return
    fi

    progress "post-restore cooldown for ${RESTORE_COOLDOWN_SECONDS}s"
    sleep "$RESTORE_COOLDOWN_SECONDS"
    sample_status post-restore-cooldown
}

stop_load() {
    if [ -n "${LOAD_PID:-}" ]; then
        kill "$LOAD_PID" >/dev/null 2>&1 || true
        wait "$LOAD_PID" 2>/dev/null || true
        LOAD_PID=""
    fi

    if [ -n "${LOAD_PIDS:-}" ]; then
        for pid in $LOAD_PIDS; do
            kill "$pid" >/dev/null 2>&1 || true
            wait "$pid" 2>/dev/null || true
        done
        LOAD_PIDS=""
    fi
}

start_builtin_load() {
    i=1
    progress "starting built-in CPU load with $LOAD_WORKERS workers"
    while [ "$i" -le "$LOAD_WORKERS" ]; do
        (while :; do :; done) &
        load_pids="${LOAD_PIDS:-}"
        LOAD_PIDS="${load_pids}${load_pids:+ }$!"
        i=$((i + 1))
    done
}

start_load() {
    case "${LOAD_CMD:-}" in
        builtin|auto)
            start_builtin_load
            return 0
            ;;
        '')
            return 1
            ;;
    esac

    progress "starting load command: $LOAD_CMD"
    sh -c "$LOAD_CMD" >/dev/null 2>"$OUT_DIR/load.stderr.log" &
    LOAD_PID="$!"
    sleep 3

    if ! kill -0 "$LOAD_PID" >/dev/null 2>&1; then
        wait "$LOAD_PID" 2>/dev/null || true
        LOAD_PID=""
        {
            printf 'load_active=false\n'
            printf 'reason=load command exited before sampling\n'
            printf 'command=%s\n' "$LOAD_CMD"
            printf 'stderr:\n'
            [ -f "$OUT_DIR/load.stderr.log" ] && cat "$OUT_DIR/load.stderr.log"
        } > "$OUT_DIR/load.failed.txt"
        progress "load command exited before sampling; skipping load phases"
        return 1
    fi

    return 0
}
