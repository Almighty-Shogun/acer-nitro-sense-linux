#!/usr/bin/env sh

run_phase() {
    label="$1"
    coolboost="$2"

    progress "setting CoolBoost $coolboost for $label"
    "$ANS" coolboost "$coolboost" > "$OUT_DIR/$label.coolboost-set.txt" 2>&1 || true
    sample_status "$label"
}

run_mode_phase() {
    label="$1"
    coolboost="$2"
    fan_mode="$3"
    profile="$4"

    progress "setting CoolBoost $coolboost, fan-mode $fan_mode, profile $profile for $label"
    if [ "$coolboost" = "off" ]; then
        "$ANS" coolboost off > "$OUT_DIR/$label.coolboost-set.txt" 2>&1 || true
        "$ANS" fan-mode "$fan_mode" > "$OUT_DIR/$label.fan-mode-set.txt" 2>&1 || true
        "$ANS" profile "$profile" > "$OUT_DIR/$label.profile-set.txt" 2>&1 || true
    else
        "$ANS" fan-mode "$fan_mode" > "$OUT_DIR/$label.fan-mode-set.txt" 2>&1 || true
        "$ANS" profile "$profile" > "$OUT_DIR/$label.profile-set.txt" 2>&1 || true
        "$ANS" coolboost on > "$OUT_DIR/$label.coolboost-set.txt" 2>&1 || true
    fi
    sample_status "$label"
}
