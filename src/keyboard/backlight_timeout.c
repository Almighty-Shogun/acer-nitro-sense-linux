#include "keyboard/backlight_timeout.h"

#include "keyboard/backlight.h"

#include <stdio.h>

void keyboard_backlight_timeout_init(const struct ans_config *cfg,
                                     daemon_runtime_state *runtime)
{
    runtime->keyboard_backlight_timeout_enabled =
        cfg->keyboard_backlight.timeout_supported &&
        cfg->keyboard_backlight.timeout_default_enabled;
    runtime->keyboard_backlight_timeout_seconds =
        cfg->keyboard_backlight.timeout_seconds;
    runtime->keyboard_backlight_timed_off = false;
    runtime->keyboard_backlight_restore_percent = 100;
}

void keyboard_backlight_timeout_note_manual_set(daemon_runtime_state *runtime,
                                                const int percent)
{
    runtime->keyboard_backlight_timed_off = false;
    if (percent > 0)
        runtime->keyboard_backlight_restore_percent = percent;
    else
        runtime->keyboard_backlight_restore_percent = 0;
}

void keyboard_backlight_timeout_tick(struct ec_device *ec,
                                     const struct ans_config *cfg,
                                     daemon_runtime_state *runtime,
                                     const int64_t now_ms,
                                     const int64_t last_activity_ms)
{
    struct keyboard_backlight_status status;
    const int64_t idle_ms = now_ms - last_activity_ms;
    const int timeout_ms = runtime->keyboard_backlight_timeout_seconds * 1000;

    if (!cfg->keyboard_backlight.available ||
        !cfg->keyboard_backlight.timeout_supported ||
        !runtime->keyboard_backlight_timeout_enabled ||
        runtime->keyboard_backlight_timeout_seconds <= 0)
        return;

    if (keyboard_backlight_read_any(ec, cfg, &status) && status.percent > 0 &&
        !runtime->keyboard_backlight_timed_off)
        runtime->keyboard_backlight_restore_percent = status.percent;

    if (idle_ms >= timeout_ms && !runtime->keyboard_backlight_timed_off) {
        if (keyboard_backlight_set_percent(ec, cfg, 0, &status)) {
            runtime->keyboard_backlight_timed_off = true;
            if (!daemon_quiet_logs)
                fprintf(stderr,
                        "keyboard_backlight_timeout state=off idle_ms=%lld restore_percent=%d\n",
                        (long long)idle_ms,
                        runtime->keyboard_backlight_restore_percent);
        }
        return;
    }

    if (idle_ms < timeout_ms && runtime->keyboard_backlight_timed_off &&
        runtime->keyboard_backlight_restore_percent > 0) {
        if (keyboard_backlight_set_percent(
                ec, cfg, runtime->keyboard_backlight_restore_percent, &status)) {
            runtime->keyboard_backlight_timed_off = false;
            if (!daemon_quiet_logs)
                fprintf(stderr,
                        "keyboard_backlight_timeout state=restored percent=%d\n",
                        status.percent);
        }
    }
}
