#include "parser.h"

#include "commands/parser/parser.h"

#include <stdio.h>
#include <string.h>

/**
 * Return whether a set command parsed into the expected target and percent.
 *
 * Set parsing validates both the fan target and the bounded percentage, so the
 * test cases keep those expectations together.
 */
static bool set_command_matches(
    const char* command,
    char* fan,
    const size_t fan_len,
    int* percent,
    const char* expected_fan,
    const int expected_percent
)
{
    const bool parsed = parse_set_command(command, fan, fan_len, percent);
    const bool fan_ok = strcmp(fan, expected_fan) == 0;

    const bool percent_ok = *percent == expected_percent;

    return parsed && fan_ok && percent_ok;
}

/**
 * Return whether malformed set commands are rejected.
 *
 * These cases cover the lower boundary, upper boundary, and trailing-token
 * parser failures with one clearly named assertion.
 */
static bool malformed_set_commands_rejected(char* fan, const size_t fan_len, int* percent)
{
    const bool low_percent_rejected = !parse_set_command("set gpu 0", fan, fan_len, percent);
    const bool high_percent_rejected = !parse_set_command("set gpu 101", fan, fan_len, percent);
    const bool extra_token_rejected = !parse_set_command("set gpu 50 extra", fan, fan_len, percent);

    return low_percent_rejected && high_percent_rejected && extra_token_rejected;
}

/**
 * Return whether a preset command parsed into the expected preset name.
 *
 * Preset command parsing is deliberately strict because names are later matched
 * against model profile entries.
 */
static bool preset_command_matches(const char* command, char* preset_name, const size_t preset_len, const char* expected_preset)
{
    const bool parsed = parse_preset_command(command, preset_name, preset_len);
    const bool preset_ok = strcmp(preset_name, expected_preset) == 0;

    return parsed && preset_ok;
}

/**
 * Return whether a power-source auto command parsed into the expected action.
 *
 * The parser only accepts the action token here; command execution decides
 * whether that action is valid for the runtime platform.
 */
static bool power_source_auto_command_matches(const char* command, char* action, const size_t action_len, const char* expected_action)
{
    const bool parsed = parse_power_source_auto_command(command, action, action_len);
    const bool action_ok = strcmp(action, expected_action) == 0;

    return parsed && action_ok;
}

/**
 * Return whether a GPU temperature command parsed into the expected action.
 *
 * GPU temperature policy commands share one parser helper, so these tests pin
 * both status and live-action command forms.
 */
static bool gpu_temp_command_matches(const char* command, char* action, const size_t action_len, const char* expected_action)
{
    const bool parsed = parse_gpu_temp_command(command, action, action_len);
    const bool action_ok = strcmp(action, expected_action) == 0;

    return parsed && action_ok;
}

/**
 * Run parser helper tests.
 *
 * The unit suite keeps this behavior pinned without requiring Acer hardware,
 * so future refactors can change structure without changing the public
 * command contract.
 */
int unit_run_parser(void)
{
    int percent, failures = 0;
    char fan[32], preset_name[32], action[32];

    const bool set_cpu_ok = set_command_matches("set cpu 45\n", fan, sizeof(fan), &percent, "cpu", 45);

    if (!set_cpu_ok)
    {
        fprintf(stderr, "unit-test failed: parse set cpu 45\n");

        failures++;
    }

    const bool set_all_ok = set_command_matches(" set all 100 ", fan, sizeof(fan), &percent, "all", 100);

    if (!set_all_ok)
    {
        fprintf(stderr, "unit-test failed: parse set all 100\n");

        failures++;
    }

    const bool malformed_set_rejected = malformed_set_commands_rejected(fan, sizeof(fan), &percent);

    if (!malformed_set_rejected)
    {
        fprintf(stderr, "unit-test failed: reject malformed set command\n");

        failures++;
    }

    const bool preset_balanced_ok = preset_command_matches("preset balanced\n", preset_name, sizeof(preset_name), "balanced");

    if (!preset_balanced_ok)
    {
        fprintf(stderr, "unit-test failed: parse preset balanced\n");

        failures++;
    }

    if (parse_preset_command("preset balanced extra", preset_name, sizeof(preset_name)))
    {
        fprintf(stderr, "unit-test failed: reject malformed preset command\n");

        failures++;
    }

    const bool power_source_auto_ok = power_source_auto_command_matches("power-source auto on\n", action, sizeof(action), "on");

    if (!power_source_auto_ok)
    {
        fprintf(stderr, "unit-test failed: parse power-source auto on\n");

        failures++;
    }

    if (parse_power_source_auto_command("power-source auto on extra", action, sizeof(action)))
    {
        fprintf(stderr, "unit-test failed: reject malformed power-source auto command\n");

        failures++;
    }

    const bool gpu_temp_live_ok = gpu_temp_command_matches("gpu-temp live\n", action, sizeof(action), "live");

    if (!gpu_temp_live_ok)
    {
        fprintf(stderr, "unit-test failed: parse gpu-temp live\n");

        failures++;
    }

    const bool gpu_temp_status_ok = gpu_temp_command_matches("gpu-temp status", action, sizeof(action), "status");

    if (!gpu_temp_status_ok)
    {
        fprintf(stderr, "unit-test failed: parse gpu-temp status\n");

        failures++;
    }

    if (parse_gpu_temp_command("gpu-temp live extra", action, sizeof(action)))
    {
        fprintf(stderr, "unit-test failed: reject malformed gpu-temp command\n");

        failures++;
    }

    return failures;
}
