#ifndef ANS_DAEMON_DIAGNOSTICS_FORMATS_H
#define ANS_DAEMON_DIAGNOSTICS_FORMATS_H

#define DIAGNOSTIC_FORMAT_FAN \
    "fan=%s name=\"%s\" sensor_group=%s control_sensor_group=%s " \
    "rpm=%d temp=%d control_temp=%d " \
    "read_register=0x%02x write_register=0x%02x " \
    "temperature_register=%s control_temperature_register=%s " \
    "reset_speed=%d missing_temperature_speed_percent=%d " \
    "curve_points=%d keep_awake=%s sensor_power_control=%s\n"

#define DIAGNOSTIC_FORMAT_SAFETY \
    "safety min_speed_percent=%d min_speed_temperature_c=%d " \
    "critical_speed_percent=%d critical_full_speed=%s " \
    "critical_step_percent=%d critical_consecutive_samples=%d " \
    "critical_release_temperature_c=%d auto_ramp_up_percent=%d " \
    "auto_ramp_bypass_temperature_c=%d missing_temperature_speed_percent=%d " \
    "max_ec_read_failures=%d max_ec_write_failures=%d\n"

#define DIAGNOSTIC_FORMAT_FAN_MODES \
    "fan_modes available=true cpu_register=0x%02x gpu_register=0x%02x " \
    "cpu_auto=0x%02x cpu_manual=0x%02x cpu_turbo=0x%02x " \
    "gpu_auto=0x%02x gpu_manual=0x%02x gpu_turbo=0x%02x\n"

#define DIAGNOSTIC_FORMAT_PLATFORM_PROFILES "platform_profiles available=true register=0x%02x default=%s profiles=%d\n"
#define DIAGNOSTIC_FORMAT_POWER_SOURCE_PROFILES "power_source_profiles available=%s auto_apply=%s ac=%s battery=%s current=%s\n"

#define FEATURE_STATUS_FORMAT_FAN_MODE \
    "fan_mode=%s cpu=%s cpu_value=0x%02x gpu=%s gpu_value=0x%02x\n"

#define FEATURE_STATUS_FORMAT_POWER_SOURCE \
    "power_source=%s policy=%s auto_apply=%s ac_profile=%s battery_profile=%s " \
    "current_profile=%s target_profile=%s\n"

#define FEATURE_STATUS_FORMAT_GPU_TEMP_READABLE \
    "gpu_temp=available policy=%s live=%s temp=%dC readable=1 source=%s reason=ok\n"

#define FEATURE_STATUS_FORMAT_GPU_TEMP_UNREADABLE \
    "gpu_temp=available policy=%s live=%s temp=-- readable=0 reason=sensor-unreadable\n"

#define FEATURE_STATUS_FORMAT_KEYBOARD_BACKLIGHT \
    "keyboard_backlight=%s timeout=%s timeout_seconds=%d timeout_backend=%s " \
    "timed_off=%s restore_percent=%d name=%s brightness=%d max_brightness=%d " \
    "percent=%d backend=%s register=%s reason=%s\n"

#define STATUS_REPLY_FORMAT_MODEL \
    "model=%s mode=%s auto=%d preset=%s coolboost=%s power_source=%s\n"

#define STATUS_REPLY_FORMAT_FAN \
    "%s rpm=%d temp=%d control=%s active_percent=%s requested=%d " \
    "effective=%d percent=%d write_value=%d safety=%s%s%s " \
    "critical_samples=%d ec_read_failures=%d ec_write_failures=%d\n"

#endif
