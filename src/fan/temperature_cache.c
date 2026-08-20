#include "fan/control.h"

#include "util/file.h"
#include "util/json.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void seed_temperatures_from_file(const char *path,
                                        const struct ans_config *cfg,
                                        fan_state states[ANS_MAX_FANS])
{
    char *json = read_text_file(path, 64 * 1024);

    if (!json)
        return;

    for (int i = 0; i < cfg->fan_len; i++) {
        char needle[64];

        snprintf(needle, sizeof(needle), "\"id\": \"%s\"", cfg->fans[i].id);

        char *fan_obj = strstr(json, needle);

        if (!fan_obj)
            continue;

        const int temp = json_int_key(fan_obj, "temp_c", -1);
        const int control_temp = json_int_key(fan_obj, "control_temp_c", temp);

        if (temp > 0 && temp < 130)
            states[i].temp_c = temp;
        if (control_temp > 0 && control_temp < 130) {
            states[i].control_temp_c = control_temp;
            states[i].control_temp_seeded = true;
        }
        if (temp > 0 && temp < 130)
            states[i].temp_seeded = true;
    }

    free(json);
}

void seed_last_temperatures(const struct ans_config *cfg,
                            fan_state states[ANS_MAX_FANS])
{
    seed_temperatures_from_file(ANS_TEMP_CACHE_PATH, cfg, states);
    seed_temperatures_from_file(ANS_STATUS_PATH, cfg, states);
}
