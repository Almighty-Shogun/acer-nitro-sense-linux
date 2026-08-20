#ifndef ANS_DAEMON_SELFTEST_COMMAND_CASES_H
#define ANS_DAEMON_SELFTEST_COMMAND_CASES_H

#include "daemon/types.h"

int selftest_run_ec_debug_commands(struct ec_device *ec,
                                   const struct ans_config *cfg,
                                   fan_state states[ANS_MAX_FANS],
                                   bool *auto_mode, char *preset,
                                   size_t preset_len,
                                   bool *coolboost_enabled);
int selftest_run_fan_socket_commands(struct ec_device *ec,
                                     const struct ans_config *cfg,
                                     fan_state states[ANS_MAX_FANS],
                                     bool *auto_mode, char *preset,
                                     size_t preset_len,
                                     bool *coolboost_enabled);
int selftest_run_firmware_auto_commands(struct ec_device *ec,
                                        const struct ans_config *cfg,
                                        fan_state states[ANS_MAX_FANS],
                                        bool *auto_mode, char *preset,
                                        size_t preset_len,
                                        bool *coolboost_enabled);
int selftest_run_invalid_and_reset_commands(struct ec_device *ec,
                                            const struct ans_config *cfg,
                                            fan_state states[ANS_MAX_FANS],
                                            bool *auto_mode, char *preset,
                                            size_t preset_len,
                                            bool *coolboost_enabled);

#endif
