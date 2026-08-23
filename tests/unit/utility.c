#include "unit/utility.h"

#include "util/json.h"
#include "util/format.h"
#include "util/number.h"
#include "config/parse.h"
#include "util/process.h"
#include "daemon/status_format.h"

#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

/**
 * Verify integer parsing expectations.
 *
 * The unit suite keeps this behavior pinned without requiring Acer hardware,
 * so future refactors can change structure without changing the public
 * command contract.
 */
static int expect_int(const char* label, const int actual, const int expected)
{
    if (actual == expected)
        return 0;

    fprintf(stderr, "unit-test failed: %s actual=%d expected=%d\n", label, actual, expected);

    return 1;
}

/**
 * Return whether invalid bounded integers are rejected.
 *
 * These cases cover overflow, trailing garbage, and values below the configured
 * minimum when no fallback should be accepted.
 */
static bool invalid_bounded_integers_rejected(int* value)
{
    const bool overflow_rejected = !parse_int_range("256", 0, 255, 0, value);
    const bool trailing_garbage_rejected = !parse_int_range("42x", 0, 255, 0, value);
    const bool below_minimum_rejected = !parse_int_range("0", 1, 100, 10, value);

    return overflow_rejected && trailing_garbage_rejected && below_minimum_rejected;
}

/**
 * Return whether firmware-controlled status logic matches expected ownership.
 *
 * Firmware control should only be reported when firmware mode is active and no
 * safety override is currently taking ownership.
 */
static bool firmware_controlled_status_matches(void)
{
    const bool firmware_owned_ok = status_fan_firmware_controlled(true, false);
    const bool safety_override_ok = !status_fan_firmware_controlled(true, true);
    const bool daemon_owned_ok = !status_fan_firmware_controlled(false, false);

    return firmware_owned_ok && safety_override_ok && daemon_owned_ok;
}

/**
 * Return whether fan control source names match the public status vocabulary.
 *
 * These strings are emitted in CLI and socket status replies, so the test keeps
 * the public names pinned.
 */
static bool fan_control_source_names_match(void)
{
    const bool firmware_ok = strcmp(status_fan_control_source(true, false), "firmware") == 0;
    const bool daemon_ok = strcmp(status_fan_control_source(false, false), "daemon") == 0;
    const bool safety_ok = strcmp(status_fan_control_source(false, true), "safety") == 0;

    return firmware_ok && daemon_ok && safety_ok;
}

/**
 * Verify numeric helper behavior.
 *
 * The unit suite keeps this behavior pinned without requiring Acer hardware,
 * so future refactors can change structure without changing the public
 * command contract.
 */
static int unit_run_number_helpers(void)
{
    int value = 0;
    int failures = 0;

    if (!parse_int_range("0x10", 0, 255, 0, &value) || value != 16)
    {
        fprintf(stderr, "unit-test failed: parse hex byte range\n");

        failures++;
    }

    const bool invalid_values_rejected = invalid_bounded_integers_rejected(&value);

    if (!invalid_values_rejected)
    {
        fprintf(stderr, "unit-test failed: reject invalid bounded integers\n");

        failures++;
    }

    return failures;
}

/**
 * Verify string helper behavior.
 *
 * The unit suite keeps this behavior pinned without requiring Acer hardware,
 * so future refactors can change structure without changing the public
 * command contract.
 */
static int unit_run_config_helpers(void)
{
    const char* json = "{ \"speed\": 42, \"enabled\": true, \"name\": \"quiet\" }";

    int value = 7;

    bool enabled = false;

    char name[16];
    int failures = 0;

    if (!config_optional_int_key(json, "missing", &value) || value != 7)
    {
        fprintf(stderr, "unit-test failed: optional int preserves missing value\n");

        failures++;
    }

    if (!config_optional_int_key(json, "speed", &value) || value != 42)
    {
        fprintf(stderr, "unit-test failed: optional int reads present value\n");

        failures++;
    }

    if (!config_optional_bool_key(json, "enabled", &enabled) || !enabled)
    {
        fprintf(stderr, "unit-test failed: optional bool reads present value\n");

        failures++;
    }

    if (!config_required_string_key(json, "name", name, sizeof(name)) || strcmp(name, "quiet") != 0)
    {
        fprintf(stderr, "unit-test failed: required string reads value\n");

        failures++;
    }

    failures += expect_int("byte value valid", config_byte_value_valid(255), 1);
    failures += expect_int("byte value invalid", config_byte_value_valid(256), 0);
    failures += expect_int("percent value valid", config_percent_value_valid(1), 1);
    failures += expect_int("percent value invalid", config_percent_value_valid(0), 0);
    failures += expect_int("speed value valid", config_speed_value_valid(0), 1);
    failures += expect_int("speed value invalid", config_speed_value_valid(101), 0);

    return failures;
}

/**
 * Verify JSON helper behavior.
 *
 * The unit suite keeps this behavior pinned without requiring Acer hardware,
 * so future refactors can change structure without changing the public
 * command contract.
 */
static int unit_run_process_helpers(void)
{
    const char* const argv[] = {"printf", "fan", NULL};
    const char* const stderr_argv[] = {"sh", "-c", "printf fan-error >&2", NULL};

    char buf[16];

    pid_t pid;

    int failures = 0;

    FILE* stream = process_open_stdout("printf", argv, &pid);

    if (!stream)
    {
        fprintf(stderr, "unit-test failed: process stdout open\n");

        return 1;
    }

    if (!fgets(buf, sizeof(buf), stream) || strcmp(buf, "fan") != 0)
    {
        fprintf(stderr, "unit-test failed: process stdout read\n");

        failures++;
    }

    const int status = process_close_stdout(stream, pid);

    if (status < 0 || !WIFEXITED(status) || WEXITSTATUS(status) != 0)
    {
        fprintf(stderr, "unit-test failed: process stdout close\n");

        failures++;
    }

    stream = process_open_output("sh", stderr_argv, true, &pid);

    if (!stream)
    {
        fprintf(stderr, "unit-test failed: process stderr open\n");

        return failures + 1;
    }

    if (!fgets(buf, sizeof(buf), stream) || strcmp(buf, "fan-error") != 0)
    {
        fprintf(stderr, "unit-test failed: process stderr read\n");

        failures++;
    }

    const int stderr_status = process_close_stdout(stream, pid);

    if (stderr_status < 0 || !WIFEXITED(stderr_status) || WEXITSTATUS(stderr_status) != 0)
    {
        fprintf(stderr, "unit-test failed: process stderr close\n");

        failures++;
    }

    return failures;
}

/**
 * Verify file helper behavior.
 *
 * The unit suite keeps this behavior pinned without requiring Acer hardware,
 * so future refactors can change structure without changing the public
 * command contract.
 */
static int unit_run_json_helpers(void)
{
    char buf[128];
    text_buffer out;
    int failures = 0;

    text_buffer_init(&out, buf, sizeof(buf));

    if (json_append_string(&out, "Acer \"Nitro\"\\GPU\n\t") < 0 || strcmp(buf, "\"Acer \\\"Nitro\\\"\\\\GPU\\n\\t\"") != 0)
    {
        fprintf(stderr, "unit-test failed: json string escaping\n");

        failures++;
    }

    text_buffer_init(&out, buf, 8);

    if (json_append_string(&out, "too long") == 0 || text_buffer_ok(&out))
    {
        fprintf(stderr, "unit-test failed: json string overflow tracking\n");

        failures++;
    }

    return failures;
}

/**
 * Verify formatting helper behavior.
 *
 * The unit suite keeps this behavior pinned without requiring Acer hardware,
 * so future refactors can change structure without changing the public
 * command contract.
 */
static int unit_run_status_helpers(void)
{
    int failures = 0;
    char active_percent[16];

    fan_state state = {.percent = 40, .requested_percent = 0};

    failures += expect_int("requested percent fallback", status_requested_percent(&state), 40);

    state.requested_percent = 55;
    failures += expect_int("requested percent explicit", status_requested_percent(&state), 55);

    const bool firmware_controlled_ok = firmware_controlled_status_matches();

    if (!firmware_controlled_ok)
    {
        fprintf(stderr, "unit-test failed: firmware controlled status\n");

        failures++;
    }

    const bool control_source_names_ok = fan_control_source_names_match();

    if (!control_source_names_ok)
    {
        fprintf(stderr, "unit-test failed: fan control source names\n");

        failures++;
    }

    status_active_percent_text(active_percent, sizeof(active_percent), true, false, 75, "firmware");

    if (strcmp(active_percent, "firmware") != 0)
    {
        fprintf(stderr, "unit-test failed: firmware active percent text\n");

        failures++;
    }

    status_active_percent_text(active_percent, sizeof(active_percent), true, true, 75, "firmware");

    if (strcmp(active_percent, "75") != 0)
    {
        fprintf(stderr, "unit-test failed: safety active percent text\n");

        failures++;
    }

    return failures;
}

/**
 * Verify utility helper behavior.
 *
 * The unit suite keeps this behavior pinned without requiring Acer hardware,
 * so future refactors can change structure without changing the public
 * command contract.
 */
int unit_run_utility_helpers(void)
{
    int failures = 0;

    failures += unit_run_number_helpers();
    failures += unit_run_config_helpers();
    failures += unit_run_process_helpers();
    failures += unit_run_json_helpers();
    failures += unit_run_status_helpers();

    return failures;
}
