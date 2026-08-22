#include "parser.h"

#include "commands/parser/parser.h"

#include <stdio.h>
#include <string.h>

int unit_run_parser(void)
{
    char fan[32];
    char preset_name[32];
    char action[32];
    int percent = 0;
    int failures = 0;

    if (!parse_set_command("set cpu 45\n", fan, sizeof(fan), &percent) ||
        strcmp(fan, "cpu") != 0 || percent != 45) {
        fprintf(stderr, "unit-test failed: parse set cpu 45\n");
        failures++;
    }

    if (!parse_set_command(" set all 100 ", fan, sizeof(fan), &percent) ||
        strcmp(fan, "all") != 0 || percent != 100) {
        fprintf(stderr, "unit-test failed: parse set all 100\n");
        failures++;
    }

    if (parse_set_command("set gpu 0", fan, sizeof(fan), &percent) ||
        parse_set_command("set gpu 101", fan, sizeof(fan), &percent) ||
        parse_set_command("set gpu 50 extra", fan, sizeof(fan), &percent)) {
        fprintf(stderr, "unit-test failed: reject malformed set command\n");
        failures++;
    }

    if (!parse_preset_command("preset balanced\n", preset_name,
                              sizeof(preset_name)) ||
        strcmp(preset_name, "balanced") != 0) {
        fprintf(stderr, "unit-test failed: parse preset balanced\n");
        failures++;
    }

    if (parse_preset_command("preset balanced extra", preset_name,
                             sizeof(preset_name))) {
        fprintf(stderr, "unit-test failed: reject malformed preset command\n");
        failures++;
    }

    if (!parse_power_source_auto_command("power-source auto on\n", action,
                                         sizeof(action)) ||
        strcmp(action, "on") != 0) {
        fprintf(stderr, "unit-test failed: parse power-source auto on\n");
        failures++;
    }

    if (parse_power_source_auto_command("power-source auto on extra", action,
                                        sizeof(action))) {
        fprintf(stderr, "unit-test failed: reject malformed power-source auto command\n");
        failures++;
    }

    if (!parse_gpu_temp_command("gpu-temp live\n", action, sizeof(action)) ||
        strcmp(action, "live") != 0) {
        fprintf(stderr, "unit-test failed: parse gpu-temp live\n");
        failures++;
    }

    if (!parse_gpu_temp_command("gpu-temp status", action, sizeof(action)) ||
        strcmp(action, "status") != 0) {
        fprintf(stderr, "unit-test failed: parse gpu-temp status\n");
        failures++;
    }

    if (parse_gpu_temp_command("gpu-temp live extra", action,
                               sizeof(action))) {
        fprintf(stderr, "unit-test failed: reject malformed gpu-temp command\n");
        failures++;
    }

    return failures;
}
