#include "daemon/status.h"

#include "daemon/status_json.h"
#include "util/file.h"
#include "util/format.h"
#include "util/json.h"

#include <time.h>

void write_status(const struct ans_config *cfg, struct ec_device *ec,
                  const fan_state states[ANS_MAX_FANS], const bool auto_mode,
                  const char *preset, const bool coolboost_enabled,
                  const hardware_names *names,
                  const daemon_runtime_state *runtime)
{
    char buf[8192];

    if (format_status_json(buf, sizeof(buf), cfg, ec, states, auto_mode,
                           preset, coolboost_enabled, names, runtime,
                           time(NULL)))
        write_text_file_atomic(ANS_STATUS_PATH, buf);
}

void write_temperature_cache(const struct ans_config *cfg, const fan_state states[ANS_MAX_FANS])
{
    char buf[512];
    text_buffer out;

    text_buffer_init(&out, buf, sizeof(buf));
    text_buffer_append(&out, "{\n  \"fans\": [\n");

    for (int i = 0; i < cfg->fan_len; i++) {
        text_buffer_append(
            &out,
            "    { \"id\": ");
        json_append_string(&out, cfg->fans[i].id);
        text_buffer_append(
            &out,
            ", \"temp_c\": %d, \"control_temp_c\": %d }%s\n",
            states[i].temp_c, states[i].control_temp_c,
            i == cfg->fan_len - 1 ? "" : ",");
    }

    text_buffer_append(&out, "  ]\n}\n");
    if (text_buffer_ok(&out))
        write_text_file_atomic(ANS_TEMP_CACHE_PATH, buf);
}
