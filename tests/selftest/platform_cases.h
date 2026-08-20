#ifndef ANS_DAEMON_SELFTEST_PLATFORM_CASES_H
#define ANS_DAEMON_SELFTEST_PLATFORM_CASES_H

#include "daemon/types.h"

int selftest_run_platform_status_commands(struct ec_device *ec,
                                          const struct ans_config *cfg,
                                          fan_state states[ANS_MAX_FANS],
                                          bool *auto_mode, char *preset,
                                          size_t preset_len,
                                          bool *coolboost_enabled);
int selftest_run_platform_fan_mode_commands(struct ec_device *ec,
                                            const struct ans_config *cfg,
                                            fan_state states[ANS_MAX_FANS],
                                            bool *auto_mode, char *preset,
                                            size_t preset_len,
                                            bool *coolboost_enabled);
int selftest_run_platform_power_source_commands(struct ec_device *ec,
                                                const struct ans_config *cfg,
                                                fan_state states[ANS_MAX_FANS],
                                                bool *auto_mode, char *preset,
                                                size_t preset_len,
                                                bool *coolboost_enabled);
int selftest_run_platform_keyboard_backlight_commands(
    struct ec_device *ec,
    const struct ans_config *cfg,
    fan_state states[ANS_MAX_FANS],
    bool *auto_mode,
    char *preset,
    size_t preset_len,
    bool *coolboost_enabled);
#endif
