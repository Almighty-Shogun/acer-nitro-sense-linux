#include "daemon/capabilities.h"

#include "config/config.h"
#include "control/protocol.h"
#include "keyboard/backlight.h"
#include "platform/power_source.h"
#include "sensors/sensors.h"
#include "util/format.h"

#include <stdio.h>
#include <string.h>

static void platform_profile_list(const struct ans_config *cfg, char *out,
                                  const size_t out_len)
{
    size_t off = 0;

    if (!cfg->platform_profiles.available || cfg->platform_profiles.profile_len == 0) {
        snprintf(out, out_len, "unavailable");
        return;
    }

    for (int i = 0; i < cfg->platform_profiles.profile_len; i++) {
        const int written = snprintf(out + off, out_len - off, "%s%s",
                                     i == 0 ? "" : ",",
                                     cfg->platform_profiles.profiles[i].id);

        if (written < 0 || (size_t)written >= out_len - off)
            break;
        off += (size_t)written;
    }
}

static void gpu_temperature_source(const struct ans_config *cfg, char *out,
                                   const size_t out_len)
{
    const struct fan_config *gpu = config_find_fan(cfg, "gpu");

    if (gpu && gpu->temperature_register >= 0) {
        snprintf(out, out_len, "ec:0x%02x", gpu->temperature_register);
        return;
    }

    snprintf(out, out_len, "hwmon");
}

static const char *keyboard_backlight_control(
    const struct ans_config *cfg,
    const struct keyboard_backlight_status *status)
{
    if (cfg->keyboard_backlight.available)
        return "ec-brightness";
    if (status->available)
        return "sysfs-brightness";
    return "unsupported";
}

void reply_capabilities(const int client, const struct ans_config *cfg,
                        const daemon_runtime_state *runtime)
{
    char profiles[256];
    char gpu_power_control[16];
    char gpu_temp_source[32];
    struct keyboard_backlight_status keyboard_backlight;
    const bool gpu_power_control_available =
        sensor_read_group_power_control("gpu", gpu_power_control,
                                        sizeof(gpu_power_control)) == 0;

    platform_profile_list(cfg, profiles, sizeof(profiles));
    gpu_temperature_source(cfg, gpu_temp_source, sizeof(gpu_temp_source));
    keyboard_backlight_read(&keyboard_backlight);

    control_reply(client, "fan_control=available modes=manual,preset,auto,firmware-auto\n");
    control_reply(client, "coolboost=%s backend=%s\n",
            availability_text(cfg->fan_modes.available),
            cfg->fan_modes.available ? "fan-mode-turbo" : "unavailable");
    control_reply(client, "fan_mode=%s modes=%s\n",
            availability_text(cfg->fan_modes.available),
            cfg->fan_modes.available ? "auto,manual,turbo" : "unavailable");
    control_reply(client, "platform_profile=%s profiles=%s\n",
            availability_text(cfg->platform_profiles.available),
            profiles);
    control_reply(client,
            "power_source_profile=%s auto_apply=%s ac_profile=%s battery_profile=%s\n",
            availability_text(power_source_profile_policy_available(cfg)),
            on_off_text(runtime && runtime->power_source_auto_apply),
            fallback_text(cfg->power_source_profiles.ac_profile, "unavailable"),
            fallback_text(cfg->power_source_profiles.battery_profile, "unavailable"));
    control_reply(client,
            "gpu_temperature=%s source=%s live_policy=%s current_policy=%s\n",
            (gpu_power_control_available ||
             strcmp(gpu_temp_source, "hwmon") != 0) ? "available" : "cached-only",
            gpu_temp_source,
            gpu_power_control_available ? "auto,live" : "unavailable",
            gpu_power_control_available ? gpu_power_control : "unavailable");
    control_reply(client,
            "keyboard_backlight=%s control=%s timeout=%s timeout_backend=%s reason=%s\n",
            availability_text(cfg->keyboard_backlight.available ||
                              keyboard_backlight.available),
            keyboard_backlight_control(cfg, &keyboard_backlight),
            supported_text(cfg->keyboard_backlight.timeout_supported),
            cfg->keyboard_backlight.timeout_supported ?
                "input-activity" : "unsupported",
            cfg->keyboard_backlight.available ? "ok" :
                keyboard_backlight_reason(&keyboard_backlight));
}
