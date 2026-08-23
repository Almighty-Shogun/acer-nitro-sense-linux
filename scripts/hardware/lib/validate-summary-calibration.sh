#!/usr/bin/env sh

write_fan_calibration_summary() {
    [ -f "$OUT_DIR/fan-calibration.status.log" ] || return 0

    printf '\nFan calibration table:\n'
    printf 'target speed cpu_avg_rpm cpu_avg_temp cpu_avg_effective cpu_avg_write_value gpu_avg_rpm gpu_avg_temp gpu_avg_effective gpu_avg_write_value samples\n'
    awk '
function val(line, key,    n,a,i) {
    n = split(line, a, " ")
    for (i = 1; i <= n; i++) {
        if (a[i] ~ "^" key "=") {
            sub("^" key "=", "", a[i])
            sub("C$", "", a[i])
            return a[i] + 0
        }
    }
    return -1
}
function strval(line, key,    n,a,i) {
    n = split(line, a, " ")
    for (i = 1; i <= n; i++) {
        if (a[i] ~ "^" key "=") {
            sub("^" key "=", "", a[i])
            return a[i]
        }
    }
    return ""
}
/^## / {
    target = strval($0, "target")
    if (target == "") target = "all"
    speed = val($0, "speed")
    key = target ":" speed
}
/^cpu / {
    cpu_rpm[key] += val($0, "rpm")
    cpu_temp[key] += val($0, "temp")
    cpu_effective[key] += val($0, "effective")
    cpu_write_value[key] += val($0, "write_value")
    cpu_n[key]++
    targets[key] = target
    speeds[key] = speed
}
/^gpu / {
    gpu_rpm[key] += val($0, "rpm")
    gpu_temp[key] += val($0, "temp")
    gpu_effective[key] += val($0, "effective")
    gpu_write_value[key] += val($0, "write_value")
    gpu_n[key]++
    targets[key] = target
    speeds[key] = speed
}
END {
    for (key in cpu_n) {
        if (cpu_n[key] > 0 && gpu_n[key] > 0)
            printf "%s %s %.0f %.1fC %.1f %.1f %.0f %.1fC %.1f %.1f %d\n",
                targets[key], speeds[key], cpu_rpm[key] / cpu_n[key],
                cpu_temp[key] / cpu_n[key],
                cpu_effective[key] / cpu_n[key],
                cpu_write_value[key] / cpu_n[key],
                gpu_rpm[key] / gpu_n[key],
                gpu_temp[key] / gpu_n[key],
                gpu_effective[key] / gpu_n[key],
                gpu_write_value[key] / gpu_n[key], cpu_n[key]
    }
}' "$OUT_DIR/fan-calibration.status.log" | sort -k1,1 -k2,2n
}
