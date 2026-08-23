#ifndef ANS_DAEMON_DIAGNOSTICS_H
#define ANS_DAEMON_DIAGNOSTICS_H

#include "daemon/types.h"

/**
 * Probe configured EC registers.
 *
 * Probe output helps validate model profiles by reading fan RPM, temperature,
 * and control registers directly.
 */
void probe_ec(struct ec_device* ec, const struct ans_config* cfg);

/**
 * Validate a model profile against the current machine.
 *
 * Validation checks DMI matching, EC access, register reads, and profile
 * safety assumptions without entering the daemon loop.
 */
void validate_model(struct ec_device* ec, const struct ans_config* cfg, const char* config_path, bool force_model);

/**
 * Print guidance for EC backend startup failures.
 *
 * The message explains which Linux kernel access paths were attempted.
 */
void print_ec_open_error(void);

#endif
