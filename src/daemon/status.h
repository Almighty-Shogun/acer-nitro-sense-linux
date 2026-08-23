#ifndef ANS_DAEMON_STATUS_H
#define ANS_DAEMON_STATUS_H

#include "daemon/types.h"
#include "hardware/hardware.h"

#include <stdbool.h>

/**
 * Write the public daemon status file.
 *
 * The status file is the low-cost read path for shell integrations and the
 * optional GNOME extension.
 */
void write_status(
    const struct ans_config* cfg,
    struct ec_device* ec,
    const fan_state states[ANS_MAX_FANS],
    bool auto_mode,
    const char* preset,
    bool coolboost_enabled,
    const hardware_names* names,
    const daemon_runtime_state* runtime
);

/**
 * Persist the latest usable temperature samples.
 *
 * Cached temperatures let status output remain useful when Linux temporarily
 * suspends a sensor such as the discrete GPU.
 */
void write_temperature_cache(const struct ans_config* cfg, const fan_state states[ANS_MAX_FANS]);

/**
 * Reply with current daemon fan status.
 *
 * This is the control-socket equivalent of the status file and is used by the
 * CLI when live daemon state is requested.
 */
void reply_status(
    int client,
    const struct ans_config* cfg,
    const fan_state states[ANS_MAX_FANS],
    bool auto_mode,
    const char* preset,
    bool coolboost_enabled
);

/**
 * Reply with the active preset context.
 *
 * The response explains whether control is manual, daemon-auto,
 * firmware-auto, or a named preset.
 */
void reply_preset_show(
    int client,
    const struct ans_config* cfg,
    const fan_state states[ANS_MAX_FANS],
    bool auto_mode,
    const char* preset
);

/**
 * Reply with configured fan presets.
 *
 * Presets are read directly from the loaded model profile.
 */
void reply_presets(int client, const struct ans_config* cfg);

/**
 * Reply with one raw EC register value.
 *
 * EC read replies are diagnostic output and remain behind control permission
 * checks in the daemon registry.
 */
void reply_ec_read(int client, struct ec_device* ec, int reg);

/**
 * Reply with a bounded raw EC register range.
 *
 * Dumps are used during model research and keep formatting centralized in the
 * status module.
 */
void reply_ec_dump(int client, struct ec_device* ec, int start, int end);

#endif
