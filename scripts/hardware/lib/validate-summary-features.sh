write_feature_summary() {
    printf 'Acer Nitro feature discovery:\n'
    if [ -f "$OUT_DIR/feature-power.txt" ]; then
        if grep -q '^powerprofilesctl=unavailable$' "$OUT_DIR/feature-power.txt"; then
            printf '  linux_power_profiles=unavailable\n'
        else
            printf '  linux_power_profiles=available\n'
        fi
        if grep -q '/sys/firmware/acpi/platform_profile: No such file' "$OUT_DIR/feature-power.txt"; then
            printf '  kernel_platform_profile=unavailable\n'
        else
            printf '  kernel_platform_profile=available_or_unknown\n'
        fi
    fi
    if [ -f "$OUT_DIR/feature-power-supply.txt" ]; then
        awk '
/type=Mains/ {kind = "mains"}
/type=Battery/ {kind = "battery"}
/online=/ && kind == "mains" {print "  ac_online=" substr($0, index($0, "=") + 1)}
/status=/ && kind == "battery" {print "  battery_status=" substr($0, index($0, "=") + 1)}
/capacity=/ && kind == "battery" {print "  battery_capacity=" substr($0, index($0, "=") + 1)}
' "$OUT_DIR/feature-power-supply.txt"
    fi
    if [ -f "$OUT_DIR/gpu-temp.txt" ]; then
        awk '
/^gpu_temp=/ {
    policy = "unknown"
    live = "unknown"
    for (i = 1; i <= NF; i++) {
        if ($i ~ /^policy=/) {
            sub(/^policy=/, "", $i)
            policy = $i
        } else if ($i ~ /^live=/) {
            sub(/^live=/, "", $i)
            live = $i
        }
    }
    printf "  gpu_temp_policy=%s\n", policy
    printf "  gpu_temp_live=%s\n", live
    exit
}
' "$OUT_DIR/gpu-temp.txt"
    fi
    if [ -f "$OUT_DIR/feature-keyboard-backlight.txt" ]; then
        if grep -q '^keyboard_backlight=available' \
            "$OUT_DIR/feature-keyboard-backlight.txt"; then
            backend="$(sed -n 's/.* backend=\([^ ]*\).*/\1/p' "$OUT_DIR/feature-keyboard-backlight.txt" | sed -n '1p')"
            timeout="$(sed -n 's/.* timeout=\([^ ]*\).*/\1/p' "$OUT_DIR/feature-keyboard-backlight.txt" | sed -n '1p')"
            printf '  keyboard_backlight=available\n'
            printf '  keyboard_backlight_backend=%s\n' "${backend:-unknown}"
            printf '  keyboard_backlight_timeout=%s\n' "${timeout:-unknown}"
        else
            printf '  keyboard_backlight=unavailable\n'
        fi
    fi
    printf '\n'
}
