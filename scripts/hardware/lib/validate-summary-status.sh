#!/usr/bin/env sh

write_status_log_summaries() {
    for file in "$OUT_DIR"/*.status.log; do
        [ -e "$file" ] || continue
        awk '
function val(line, key,    n,a,i) {
    n = split(line, a, " ")
    for (i = 1; i <= n; i++) {
        if (a[i] ~ "^" key "=") {
            sub("^" key "=", "", a[i])
            if (a[i] == "--")
                return -1
            sub("C$", "", a[i])
            return a[i] + 0
        }
    }
    return -1
}
/^# label=/ {label = $2; sub("label=", "", label)}
/^cpu / {
    rpm = val($0, "rpm")
    temp = val($0, "temp")
    percent = val($0, "percent")
    requested = val($0, "requested")
    effective = val($0, "effective")
    write_value = val($0, "write_value")
    critical_samples = val($0, "critical_samples")
    if (requested < 0) requested = percent
    if (effective < 0) effective = percent
    safety_ok = index($0, "safety=ok") > 0
    cpu_rpm += rpm
    if (temp >= 0) {
        cpu_temp += temp
        if (cpu_temp_n == 0 || temp < cpu_temp_min) cpu_temp_min = temp
        if (cpu_temp_n == 0 || temp > cpu_temp_max) cpu_temp_max = temp
        cpu_temp_n++
    }
    cpu_percent += percent
    cpu_requested += requested
    cpu_effective += effective
    if (write_value >= 0) cpu_write_value += write_value
    if (cpu_n == 0 || rpm < cpu_rpm_min) cpu_rpm_min = rpm
    if (cpu_n == 0 || rpm > cpu_rpm_max) cpu_rpm_max = rpm
    if (!safety_ok) cpu_safety++
    if (critical_samples > cpu_critical_samples_max) cpu_critical_samples_max = critical_samples
    cpu_n++
}
/^gpu / {
    rpm = val($0, "rpm")
    temp = val($0, "temp")
    percent = val($0, "percent")
    requested = val($0, "requested")
    effective = val($0, "effective")
    write_value = val($0, "write_value")
    critical_samples = val($0, "critical_samples")
    if (requested < 0) requested = percent
    if (effective < 0) effective = percent
    safety_ok = index($0, "safety=ok") > 0
    gpu_rpm += rpm
    if (temp >= 0) {
        gpu_temp += temp
        if (gpu_temp_n == 0 || temp < gpu_temp_min) gpu_temp_min = temp
        if (gpu_temp_n == 0 || temp > gpu_temp_max) gpu_temp_max = temp
        gpu_temp_n++
    }
    gpu_percent += percent
    gpu_requested += requested
    gpu_effective += effective
    if (write_value >= 0) gpu_write_value += write_value
    if (gpu_n == 0 || rpm < gpu_rpm_min) gpu_rpm_min = rpm
    if (gpu_n == 0 || rpm > gpu_rpm_max) gpu_rpm_max = rpm
    if (!safety_ok) gpu_safety++
    if (critical_samples > gpu_critical_samples_max) gpu_critical_samples_max = critical_samples
    gpu_n++
}
/^control fan=cpu / {
    control_temp = val($0, "control_temp")
    control_sensor_temp = val($0, "control_sensor_temp")
    if (control_temp >= 0) {
        cpu_control_temp += control_temp
        if (cpu_control_n == 0 || control_temp < cpu_control_temp_min) cpu_control_temp_min = control_temp
        if (cpu_control_n == 0 || control_temp > cpu_control_temp_max) cpu_control_temp_max = control_temp
        cpu_control_n++
    }
    if (control_sensor_temp >= 0) {
        cpu_control_sensor_temp += control_sensor_temp
        cpu_control_sensor_n++
    }
}
/^control fan=gpu / {
    control_temp = val($0, "control_temp")
    control_sensor_temp = val($0, "control_sensor_temp")
    if (control_temp >= 0) {
        gpu_control_temp += control_temp
        if (gpu_control_n == 0 || control_temp < gpu_control_temp_min) gpu_control_temp_min = control_temp
        if (gpu_control_n == 0 || control_temp > gpu_control_temp_max) gpu_control_temp_max = control_temp
        gpu_control_n++
    }
    if (control_sensor_temp >= 0) {
        gpu_control_sensor_temp += control_sensor_temp
        gpu_control_sensor_n++
    }
}
END {
    if (cpu_control_n == 0) {
        cpu_control_n = cpu_n
        cpu_control_temp = cpu_temp
        cpu_control_temp_min = cpu_temp_min
        cpu_control_temp_max = cpu_temp_max
    }
    if (gpu_control_n == 0) {
        gpu_control_n = gpu_n
        gpu_control_temp = gpu_temp
        gpu_control_temp_min = gpu_temp_min
        gpu_control_temp_max = gpu_temp_max
    }
    if (cpu_control_sensor_n == 0) {
        cpu_control_sensor_n = cpu_control_n
        cpu_control_sensor_temp = cpu_control_temp
    }
    if (gpu_control_sensor_n == 0) {
        gpu_control_sensor_n = gpu_control_n
        gpu_control_sensor_temp = gpu_control_temp
    }
    cpu_avg_temp = cpu_temp_n > 0 ? sprintf("%.1fC", cpu_temp / cpu_temp_n) : "unavailable"
    cpu_min_temp = cpu_temp_n > 0 ? sprintf("%dC", cpu_temp_min) : "unavailable"
    cpu_max_temp = cpu_temp_n > 0 ? sprintf("%dC", cpu_temp_max) : "unavailable"
    gpu_avg_temp = gpu_temp_n > 0 ? sprintf("%.1fC", gpu_temp / gpu_temp_n) : "unavailable"
    gpu_min_temp = gpu_temp_n > 0 ? sprintf("%dC", gpu_temp_min) : "unavailable"
    gpu_max_temp = gpu_temp_n > 0 ? sprintf("%dC", gpu_temp_max) : "unavailable"
    if (cpu_n > 0 && gpu_n > 0)
        printf "%s cpu_avg_rpm=%.0f cpu_rpm_min=%d cpu_rpm_max=%d cpu_avg_temp=%s cpu_temp_min=%s cpu_temp_max=%s cpu_avg_control_temp=%.1fC cpu_control_temp_min=%dC cpu_control_temp_max=%dC cpu_avg_control_sensor_temp=%.1fC cpu_avg_requested=%.1f cpu_avg_effective=%.1f cpu_avg_percent=%.1f cpu_avg_write_value=%.1f cpu_safety_samples=%d cpu_critical_samples_max=%d gpu_avg_rpm=%.0f gpu_rpm_min=%d gpu_rpm_max=%d gpu_avg_temp=%s gpu_temp_min=%s gpu_temp_max=%s gpu_avg_control_temp=%.1fC gpu_control_temp_min=%dC gpu_control_temp_max=%dC gpu_avg_control_sensor_temp=%.1fC gpu_avg_requested=%.1f gpu_avg_effective=%.1f gpu_avg_percent=%.1f gpu_avg_write_value=%.1f gpu_safety_samples=%d gpu_critical_samples_max=%d samples=%d\n",
            label, cpu_rpm / cpu_n, cpu_rpm_min, cpu_rpm_max,
            cpu_avg_temp, cpu_min_temp, cpu_max_temp,
            cpu_control_temp / cpu_control_n, cpu_control_temp_min,
            cpu_control_temp_max, cpu_control_sensor_temp / cpu_control_sensor_n,
            cpu_requested / cpu_n, cpu_effective / cpu_n, cpu_percent / cpu_n,
            cpu_write_value / cpu_n, cpu_safety, cpu_critical_samples_max,
            gpu_rpm / gpu_n, gpu_rpm_min, gpu_rpm_max, gpu_avg_temp,
            gpu_min_temp, gpu_max_temp, gpu_control_temp / gpu_control_n,
            gpu_control_temp_min, gpu_control_temp_max,
            gpu_control_sensor_temp / gpu_control_sensor_n, gpu_requested / gpu_n,
            gpu_effective / gpu_n, gpu_percent / gpu_n, gpu_write_value / gpu_n,
            gpu_safety, gpu_critical_samples_max, cpu_n
}' "$file"
    done
}
