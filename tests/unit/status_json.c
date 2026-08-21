#include "unit/status_json.h"

#include "daemon/status_json.h"
#include "ec/ec.h"
#include "hardware/hardware.h"

#include <stdio.h>
#include <string.h>

int unit_run_status_json(struct ec_device *ec, struct ans_config *cfg,
                         fan_state states[ANS_MAX_FANS])
{
    struct ans_config local_cfg = *cfg;
    fan_state local_states[ANS_MAX_FANS];
    char json[8192];
    hardware_names names;
    daemon_runtime_state runtime = {
        .power_source_auto_apply = true,
        .keyboard_backlight_timeout_enabled = true,
    };
    int failures = 0;

    memcpy(local_states, states, sizeof(local_states));

    snprintf(local_cfg.model, sizeof(local_cfg.model), "Acer \"Nitro\"\\Test");
    snprintf(local_cfg.fans[0].name, sizeof(local_cfg.fans[0].name),
             "CPU \"Fan\"");
    snprintf(local_cfg.fans[1].name, sizeof(local_cfg.fans[1].name),
             "GPU \\ Fan");
    snprintf(names.cpu, sizeof(names.cpu), "Intel\nCore");
    snprintf(names.gpu, sizeof(names.gpu), "NVIDIA\tGPU");
    snprintf(local_states[0].safety_reason, sizeof(local_states[0].safety_reason),
             "critical\treason");
    local_states[0].safety_active = true;

    ec_write_byte(ec, local_cfg.fan_modes.cpu_reg,
                  local_cfg.fan_modes.cpu_auto_value);
    ec_write_byte(ec, local_cfg.fan_modes.gpu_reg,
                  local_cfg.fan_modes.gpu_auto_value);
    ec_write_byte(ec, local_cfg.platform_profiles.reg,
                  local_cfg.platform_profiles.profiles[1].value);
    ec_write_byte(ec, local_cfg.keyboard_backlight.reg, 2);

    if (!format_status_json(json, sizeof(json), &local_cfg, ec, local_states, false,
                            "manual", false, &names, &runtime, 42)) {
        fprintf(stderr, "unit-test failed: status json formatting\n");
        return 1;
    }

    if (!strstr(json, "\"model\": \"Acer \\\"Nitro\\\"\\\\Test\"")) {
        fprintf(stderr, "unit-test failed: status json escapes model\n");
        failures++;
    }
    if (!strstr(json, "\"name\": \"CPU \\\"Fan\\\"\"") ||
        !strstr(json, "\"name\": \"GPU \\\\ Fan\"")) {
        fprintf(stderr, "unit-test failed: status json escapes fan names\n");
        failures++;
    }
    if (!strstr(json, "\"component_name\": \"Intel\\nCore\"") ||
        !strstr(json, "\"component_name\": \"NVIDIA\\tGPU\"")) {
        fprintf(stderr, "unit-test failed: status json escapes component names\n");
        failures++;
    }
    if (!strstr(json, "\"safety_reason\": \"critical\\treason\"")) {
        fprintf(stderr, "unit-test failed: status json escapes safety reason\n");
        failures++;
    }
    if (!strstr(json, "\"timestamp\": 42")) {
        fprintf(stderr, "unit-test failed: status json timestamp\n");
        failures++;
    }

    return failures;
}
