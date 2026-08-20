#ifndef ANS_H
#define ANS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define ANS_MAX_FANS 2
#define ANS_MAX_THRESHOLDS 32
#define ANS_MAX_WRITES 8
#define ANS_MAX_PRESETS 16
#define ANS_MAX_PLATFORM_PROFILES 8

#define ANS_RUN_DIR "/run/acer-nitro-sense"
#define ANS_SOCKET_PATH ANS_RUN_DIR "/control.sock"
#define ANS_STATUS_PATH ANS_RUN_DIR "/status.json"
#define ANS_TEMP_CACHE_PATH ANS_RUN_DIR "/temperature-cache.json"
#define ANS_STATE_DIR "/var/lib/acer-nitro-sense"
#define ANS_STATE_PATH ANS_STATE_DIR "/state.json"
#define ANS_DEFAULT_CONFIG "/etc/acer-nitro-sense/model.json"
#define ANS_MODEL_DIR "/usr/share/acer-nitro-sense/models"
#define ANS_FALLBACK_CONFIG "models/acer-nitro-an517-51.json"
#define ANS_CONTROL_GROUP "acer-nitro-sense"

struct threshold {
    int up;
    int down;
    int speed;
};

struct fan_config {
    char id[16];
    char name[64];
    char sensor_group[16];
    char control_sensor_group[16];
    char sensor_power_control[8];
    bool keep_awake;
    int read_register;
    int write_register;
    int temperature_register;
    int control_temperature_register;
    int read_min;
    int read_max;
    int write_min;
    int write_max;
    int reset_speed;
    int missing_temperature_speed_percent;
    struct threshold curve[ANS_MAX_THRESHOLDS];
    int curve_len;
};

struct ec_write_config {
    int reg;
    int value;
    int reset_value;
};

struct preset_config {
    char id[32];
    int cpu;
    int gpu;
};

struct safety_config {
    int min_speed_percent;
    int min_speed_temperature_c;
    int critical_speed_percent;
    bool critical_full_speed;
    int critical_step_percent;
    int critical_consecutive_samples;
    int critical_release_temperature_c;
    int auto_ramp_up_percent;
    int auto_ramp_bypass_temperature_c;
    int missing_temperature_speed_percent;
    int max_ec_read_failures;
    int max_ec_write_failures;
};

struct coolboost_config {
    bool default_enabled;
};

struct fan_mode_config {
    bool available;
    int cpu_reg;
    int gpu_reg;
    int cpu_auto_value;
    int cpu_manual_value;
    int cpu_turbo_value;
    int gpu_auto_value;
    int gpu_manual_value;
    int gpu_turbo_value;
};

struct platform_profile_entry {
    char id[32];
    int value;
};

struct platform_profile_config {
    bool available;
    int reg;
    char default_profile[32];
    struct platform_profile_entry profiles[ANS_MAX_PLATFORM_PROFILES];
    int profile_len;
};

struct power_source_profile_config {
    bool auto_apply;
    char ac_profile[32];
    char battery_profile[32];
};

struct keyboard_backlight_config {
    bool available;
    int reg;
    int min_value;
    int max_value;
    bool timeout_supported;
    bool timeout_default_enabled;
    int timeout_seconds;
};

struct ans_config {
    char model[96];
    char default_preset[32];
    char ec_path[256];
    char allowed_dmi[8][64];
    int allowed_dmi_len;
    int poll_interval_ms;
    int critical_temperature_c;
    bool read_words;
    struct safety_config safety;
    struct coolboost_config coolboost;
    struct fan_mode_config fan_modes;
    struct platform_profile_config platform_profiles;
    struct power_source_profile_config power_source_profiles;
    struct keyboard_backlight_config keyboard_backlight;
    struct ec_write_config init_writes[ANS_MAX_WRITES];
    int init_write_len;
    struct fan_config fans[ANS_MAX_FANS];
    int fan_len;
    struct preset_config presets[ANS_MAX_PRESETS];
    int preset_len;
};

enum ec_backend {
    EC_BACKEND_NONE = 0,
    EC_BACKEND_EC_SYS,
    EC_BACKEND_ACPI_EC,
    EC_BACKEND_IO_PORTS,
    EC_BACKEND_FAKE,
};

struct ec_device {
    enum ec_backend backend;
    int fd;
    char name[32];
    uint8_t fake_regs[256];
};

int config_load(const char *path, struct ans_config *cfg);
const struct preset_config *config_find_preset(const struct ans_config *cfg, const char *id);
const struct platform_profile_entry *config_find_platform_profile(const struct ans_config *cfg, const char *id);
const struct fan_config *config_find_fan(const struct ans_config *cfg, const char *id);

int ec_open_rw(const char *path, struct ec_device *ec);
void ec_close(struct ec_device *ec);
int ec_read_byte(struct ec_device *ec, int reg);
int ec_write_byte(struct ec_device *ec, int reg, int value);
int ec_read_word(struct ec_device *ec, int reg);

int sensor_read_group_max_c(const char *group);
int sensor_set_group_power_control(const char *group, const char *control);
int sensor_read_group_power_control(const char *group, char *out, size_t out_len);

int mkdir_p(const char *path);
char *read_text_file(const char *path, size_t limit);
int write_text_file_atomic(const char *path, const char *content);
bool string_contains_case(const char *haystack, const char *needle);
void trim_ascii(char *s);
int clamp_int(int value, int min, int max);

#endif
