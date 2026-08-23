#ifndef ANS_DAEMON_UNIT_COMMAND_CASES_H
#define ANS_DAEMON_UNIT_COMMAND_CASES_H

#include "daemon/types.h"

/**
 * Run EC debug command routing tests.
 *
 * These cases verify permission handling and bounded register access for raw
 * EC read and dump commands.
 */
int unit_run_ec_debug_commands(
    struct ec_device* ec,
    const struct ans_config* cfg,
    fan_state states[ANS_MAX_FANS],
    bool* auto_mode,
    char* preset,
    size_t preset_len,
    bool* coolboost_enabled
);

/**
 * Run daemon lifecycle command tests.
 *
 * These cases cover resume and stop behavior through the control-command
 * dispatcher.
 */
int unit_run_daemon_lifecycle_commands(
    struct ec_device* ec,
    const struct ans_config* cfg,
    fan_state states[ANS_MAX_FANS],
    bool* auto_mode,
    char* preset,
    size_t preset_len,
    bool* coolboost_enabled
);

/**
 * Run fan control socket command tests.
 *
 * These cases verify manual fan, preset, status, and permission behavior at
 * the daemon command boundary.
 */
int unit_run_fan_socket_commands(
    struct ec_device* ec,
    const struct ans_config* cfg,
    fan_state states[ANS_MAX_FANS],
    bool* auto_mode,
    char* preset,
    size_t preset_len,
    bool* coolboost_enabled
);

/**
 * Run firmware-auto command tests.
 *
 * These cases verify that firmware fan-mode writes and persisted state stay in
 * sync.
 */
int unit_run_firmware_auto_commands(
    struct ec_device* ec,
    const struct ans_config* cfg,
    fan_state states[ANS_MAX_FANS],
    bool* auto_mode,
    char* preset,
    size_t preset_len,
    bool* coolboost_enabled
);

/**
 * Run invalid command and reset-path tests.
 *
 * These cases ensure bad commands return deterministic errors and reset writes
 * keep firmware state recoverable.
 */
int unit_run_invalid_and_reset_commands(
    struct ec_device* ec,
    const struct ans_config* cfg,
    fan_state states[ANS_MAX_FANS],
    bool* auto_mode,
    char* preset,
    size_t preset_len,
    bool* coolboost_enabled
);

#endif
