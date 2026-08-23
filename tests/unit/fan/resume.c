#include "cases.h"

#include "ec/ec.h"
#include "../fixture.h"
#include "../helpers.h"
#include "fan/control.h"
#include "daemon/status.h"

#include <stdio.h>
#include <string.h>

/**
 * Return whether the fake EC registers contain the expected fan values.
 *
 * Resume behavior is validated through the same write registers used by the
 * daemon, so the tests verify the EC-facing side effects directly.
 */
static bool fan_write_registers_match(
    struct ec_device* ec,
    const struct ans_config* cfg,
    const int cpu_value,
    const int gpu_value
)
{
    const bool cpu_write_ok = ec_read_byte(ec, cfg->fans[0].write_register) == cpu_value;
    const bool gpu_write_ok = ec_read_byte(ec, cfg->fans[1].write_register) == gpu_value;

    return cpu_write_ok && gpu_write_ok;
}

/**
 * Return whether applying the balanced preset produced its configured writes.
 *
 * This pins the preset setup step separately from the resume reapply step, so
 * failures identify which side of the behavior changed.
 */
static bool balanced_preset_writes_match(struct ec_device* ec, const struct ans_config* cfg, fan_state states[ANS_MAX_FANS])
{
    const bool preset_applied = apply_preset(ec, cfg, states, "balanced");
    const bool writes_ok = fan_write_registers_match(ec, cfg, 50, 45);

    return preset_applied && writes_ok;
}

/**
 * Return whether the preset-show reply describes the restored manual state.
 *
 * The public reply text is part of the CLI contract, so this test checks the
 * important fields without depending on the full multi-line status payload.
 */
static bool preset_show_reply_matches(
    const struct ans_config* cfg,
    fan_state states[ANS_MAX_FANS],
    char* reply,
    const size_t reply_len
)
{
    const bool reply_read = unit_read_reply(reply_preset_show, cfg, states, false, "manual", reply, reply_len) >= 0;

    const bool mode_ok = strstr(reply, "mode=manual preset=manual") != NULL;
    const bool cpu_ok = strstr(reply, "cpu requested=70 effective=70 percent=70") != NULL;

    return reply_read && mode_ok && cpu_ok;
}

/**
 * Verify preset resume status.
 *
 * The unit suite keeps this behavior pinned without requiring Acer hardware,
 * so future refactors can change structure without changing the public
 * command contract.
 */
int unit_run_fan_preset_resume_status(struct ec_device* ec, const struct ans_config* cfg, fan_state states[ANS_MAX_FANS])
{
    int failures = 0;
    char reply[1024];

    reset_unit_test_states(cfg, states);

    const bool preset_writes_ok = balanced_preset_writes_match(ec, cfg, states);

    if (!preset_writes_ok)
    {
        fprintf(stderr, "unit-test failed: fake EC preset writes\n");

        failures++;
    }

    states[0].percent = 70;
    states[1].percent = 65;
    states[0].requested_percent = 70;
    states[1].requested_percent = 65;

    apply_current_control_state(ec, cfg, states);

    const bool resume_writes_ok = fan_write_registers_match(ec, cfg, 70, 65);

    if (!resume_writes_ok)
    {
        fprintf(stderr, "unit-test failed: resume reapply writes\n");

        failures++;
    }

    const bool preset_reply_ok = preset_show_reply_matches(cfg, states, reply, sizeof(reply));

    if (!preset_reply_ok)
    {
        fprintf(stderr, "preset show reply:\n%s", reply);
        fprintf(stderr, "unit-test failed: preset show response format\n");

        failures++;
    }

    return failures;
}
