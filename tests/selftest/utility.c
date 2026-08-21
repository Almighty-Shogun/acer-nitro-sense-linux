#include "selftest/utility.h"

#include "config/parse.h"
#include "daemon/status_format.h"
#include "util/number.h"

#include <stdio.h>
#include <string.h>

static int expect_int(const char *label, const int actual, const int expected)
{
    if (actual == expected)
        return 0;

    fprintf(stderr, "self-test failed: %s actual=%d expected=%d\n",
            label, actual, expected);
    return 1;
}

static int selftest_run_number_helpers(void)
{
    int value = 0;
    int failures = 0;

    if (!parse_int_range("0x10", 0, 255, 0, &value) || value != 16) {
        fprintf(stderr, "self-test failed: parse hex byte range\n");
        failures++;
    }

    if (parse_int_range("256", 0, 255, 0, &value) ||
        parse_int_range("42x", 0, 255, 0, &value) ||
        parse_int_range("0", 1, 100, 10, &value)) {
        fprintf(stderr, "self-test failed: reject invalid bounded integers\n");
        failures++;
    }

    return failures;
}

static int selftest_run_config_helpers(void)
{
    const char *json = "{ \"speed\": 42, \"enabled\": true, \"name\": \"quiet\" }";
    int value = 7;
    bool enabled = false;
    char name[16];
    int failures = 0;

    if (!config_optional_int_key(json, "missing", &value) || value != 7) {
        fprintf(stderr, "self-test failed: optional int preserves missing value\n");
        failures++;
    }

    if (!config_optional_int_key(json, "speed", &value) || value != 42) {
        fprintf(stderr, "self-test failed: optional int reads present value\n");
        failures++;
    }

    if (!config_optional_bool_key(json, "enabled", &enabled) || !enabled) {
        fprintf(stderr, "self-test failed: optional bool reads present value\n");
        failures++;
    }

    if (!config_required_string_key(json, "name", name, sizeof(name)) ||
        strcmp(name, "quiet") != 0) {
        fprintf(stderr, "self-test failed: required string reads value\n");
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

static int selftest_run_status_helpers(void)
{
    fan_state state = {.percent = 40, .requested_percent = 0};
    char active_percent[16];
    int failures = 0;

    failures += expect_int("requested percent fallback",
                           status_requested_percent(&state), 40);

    state.requested_percent = 55;
    failures += expect_int("requested percent explicit",
                           status_requested_percent(&state), 55);

    if (!status_fan_firmware_controlled(true, false) ||
        status_fan_firmware_controlled(true, true) ||
        status_fan_firmware_controlled(false, false)) {
        fprintf(stderr, "self-test failed: firmware controlled status\n");
        failures++;
    }

    if (strcmp(status_fan_control_source(true, false), "firmware") != 0 ||
        strcmp(status_fan_control_source(false, false), "daemon") != 0 ||
        strcmp(status_fan_control_source(false, true), "safety") != 0) {
        fprintf(stderr, "self-test failed: fan control source names\n");
        failures++;
    }

    status_active_percent_text(active_percent, sizeof(active_percent),
                               true, false, 75, "firmware");
    if (strcmp(active_percent, "firmware") != 0) {
        fprintf(stderr, "self-test failed: firmware active percent text\n");
        failures++;
    }

    status_active_percent_text(active_percent, sizeof(active_percent),
                               true, true, 75, "firmware");
    if (strcmp(active_percent, "75") != 0) {
        fprintf(stderr, "self-test failed: safety active percent text\n");
        failures++;
    }

    return failures;
}

int selftest_run_utility_helpers(void)
{
    int failures = 0;

    failures += selftest_run_number_helpers();
    failures += selftest_run_config_helpers();
    failures += selftest_run_status_helpers();

    return failures;
}
