#ifndef ANS_UNIT_FIXTURE_H
#define ANS_UNIT_FIXTURE_H

#include "daemon/types.h"

/**
 * Initialize the shared unit-test model profile.
 *
 * Tests use one deterministic profile so EC registers, fan curves, and feature
 * flags stay predictable.
 */
void init_unit_test_config(struct ans_config* cfg);

/**
 * Reset shared fan state for a test case.
 *
 * Each case starts from a clean telemetry and safety state while keeping the
 * same model profile.
 */
void reset_unit_test_states(const struct ans_config* cfg, fan_state states[ANS_MAX_FANS]);

#endif
