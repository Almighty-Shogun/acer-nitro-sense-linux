#include "hardware/hardware.h"

#include "config/types.h"
#include "util/file.h"
#include "util/string.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void append_model_part(char *model, const size_t model_len,
                              const char *text)
{
    const size_t used = strlen(model);

    if (used >= model_len - 1)
        return;

    snprintf(model + used, model_len - used, "%s%s", used > 0 ? " " : "", text);
}

const char *load_dmi_model(void)
{
    static char model[256];
    const char *paths[] = {
        "/sys/class/dmi/id/product_name",
        "/sys/class/dmi/id/board_name",
    };

    model[0] = '\0';
    for (size_t i = 0; i < sizeof(paths) / sizeof(paths[0]); i++) {
        char *text = read_text_file(paths[i], 256);

        if (!text)
            continue;

        trim_ascii(text);
        if (text[0] == '\0') {
            free(text);
            continue;
        }
        append_model_part(model, sizeof(model), text);
        free(text);
    }

    trim_ascii(model);

    return model;
}

bool dmi_allowed(const struct ans_config *cfg, const char *dmi)
{
    if (cfg->allowed_dmi_len == 0)
        return true;

    for (int i = 0; i < cfg->allowed_dmi_len; i++) {
        if (string_contains_case(dmi, cfg->allowed_dmi[i]))
            return true;
    }

    return false;
}
