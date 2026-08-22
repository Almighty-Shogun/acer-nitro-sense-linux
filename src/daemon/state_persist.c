#include "daemon/state.h"

#include "fan/control.h"
#include "platform/control.h"
#include "util/file.h"
#include "util/format.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

void write_control_state(const struct ans_config *cfg,
                         const fan_state states[ANS_MAX_FANS],
                         const bool auto_mode, const char *preset,
                         const bool coolboost_enabled,
                         const daemon_runtime_state *runtime)
{
    char buf[1024];
    text_buffer out;
    const time_t now = time(NULL);

    (void)coolboost_enabled;

    if (!daemon_persist_control_state)
        return;

    if (mkdir_p(ANS_STATE_DIR) < 0) {
        fprintf(stderr, "warning: failed to create state directory: %s\n",
                strerror(errno));
        return;
    }

    text_buffer_init(&out, buf, sizeof(buf));
    text_buffer_append(&out,
                       "{\n  \"schema\": 1,\n  \"mode\": \"%s\",\n"
                       "  \"auto\": %s,\n  \"preset\": \"%s\",\n"
                       "  \"coolboost\": false,\n"
                       "  \"power_source_auto_apply\": %s,\n"
                       "  \"keyboard_backlight_timeout_enabled\": %s,\n"
                       "  \"timestamp\": %ld,\n  \"fans\": [\n",
                       control_mode(auto_mode, preset),
                       auto_mode ? "true" : "false",
                       preset,
                       runtime && runtime->power_source_auto_apply ?
                           "true" : "false",
                       runtime && runtime->keyboard_backlight_timeout_enabled ?
                           "true" : "false",
                       (long)now);

    for (int i = 0; i < cfg->fan_len; i++) {
        const int percent = states[i].requested_percent > 0 ?
            states[i].requested_percent : states[i].percent;

        text_buffer_append(&out,
                           "    { \"id\": \"%s\", \"percent\": %d }%s\n",
                           cfg->fans[i].id, percent,
                           i == cfg->fan_len - 1 ? "" : ",");
    }

    text_buffer_append(&out, "  ]\n}\n");

    if (!text_buffer_ok(&out)) {
        fprintf(stderr, "warning: failed to save control state: buffer too small\n");
        return;
    }

    if (write_text_file_atomic(ANS_STATE_PATH, buf) < 0)
        fprintf(stderr, "warning: failed to save control state: %s\n",
                strerror(errno));
}
