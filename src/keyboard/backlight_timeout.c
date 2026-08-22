#include "keyboard/backlight_timeout.h"

#include "keyboard/backlight.h"

#include <stdio.h>

#define KEYBOARD_BACKLIGHT_TIMEOUT_OFF_LOG_FORMAT \
    "keyboard_backlight_timeout state=off idle_ms=%lld restore_percent=%d\n"

#define KEYBOARD_BACKLIGHT_TIMEOUT_RESTORED_LOG_FORMAT \
    "keyboard_backlight_timeout state=restored percent=%d\n"

/**
 * Return whether keyboard backlight timeout can run.
 *
 * Timeout needs both model support and a runtime opt-in. Unsupported systems
 * keep keyboard lighting untouched.
 */
static bool keyboard_backlight_timeout_can_run(const struct ans_config* cfg, const daemon_runtime_state* runtime)
{
    return cfg->keyboard_backlight.available
        && cfg->keyboard_backlight.timeout_supported
        && runtime->keyboard_backlight_timeout_enabled
        && runtime->keyboard_backlight_timeout_seconds > 0;
}

/**
 * Return whether the backlight should be turned off for idle timeout.
 *
 * This check is kept separate from the EC write so the timeout state machine is
 * readable at the call site.
 */
static bool keyboard_backlight_should_turn_off(const daemon_runtime_state* runtime, const int64_t idle_ms, const int timeout_ms)
{
    return idle_ms >= timeout_ms && !runtime->keyboard_backlight_timed_off;
}

/**
 * Return whether the backlight should be restored after activity resumes.
 *
 * Restore only happens when the daemon previously timed the keyboard off and a
 * positive remembered brightness is available.
 */
static bool keyboard_backlight_should_restore(const daemon_runtime_state* runtime, const int64_t idle_ms, const int timeout_ms)
{
    return idle_ms < timeout_ms && runtime->keyboard_backlight_timed_off && runtime->keyboard_backlight_restore_percent > 0;
}

/**
 * Remember the current backlight level as the restore target.
 *
 * The restore value is updated only while the timeout has not dimmed the
 * keyboard, otherwise the timed-off zero state would overwrite the user's
 * previous brightness.
 */
static void keyboard_backlight_update_restore_percent(
    struct ec_device* ec,
    const struct ans_config* cfg,
    daemon_runtime_state* runtime,
    struct keyboard_backlight_status* status
)
{
    if (runtime->keyboard_backlight_timed_off) return;

    if (keyboard_backlight_read_any(ec, cfg, status) && status->percent > 0)
        runtime->keyboard_backlight_restore_percent = status->percent;
}

/**
 * Log that the timeout turned the keyboard backlight off.
 *
 * The restore value is included so hardware timeout behavior can be diagnosed
 * from daemon logs without extra state dumps.
 */
static void keyboard_backlight_log_timed_off(const int64_t idle_ms, const int restore_percent)
{
    if (daemon_quiet_logs) return;

    fprintf(
        stderr,
        KEYBOARD_BACKLIGHT_TIMEOUT_OFF_LOG_FORMAT,
        (long long)idle_ms,
        restore_percent
    );
}

/**
 * Log that activity restored the keyboard backlight.
 *
 * The logged percentage is the value reported after the EC write, not only the
 * requested restore target.
 */
static void keyboard_backlight_log_restored(const struct keyboard_backlight_status* status)
{
    if (daemon_quiet_logs) return;

    fprintf(stderr, KEYBOARD_BACKLIGHT_TIMEOUT_RESTORED_LOG_FORMAT, status->percent);
}

/**
 * Initialize keyboard backlight timeout state.
 *
 * The model default decides whether timeout starts enabled; runtime commands
 * can still change it after startup.
 */
void keyboard_backlight_timeout_init(const struct ans_config* cfg, daemon_runtime_state* runtime)
{
    runtime->keyboard_backlight_timeout_enabled = cfg->keyboard_backlight.timeout_supported
        && cfg->keyboard_backlight.timeout_default_enabled;

    runtime->keyboard_backlight_timeout_seconds = cfg->keyboard_backlight.timeout_seconds;
    runtime->keyboard_backlight_timed_off = false;
    runtime->keyboard_backlight_restore_percent = 100;
}

/**
 * Record a manual keyboard backlight change.
 *
 * A user-initiated brightness change clears the timed-off state and becomes the
 * next restore target.
 */
void keyboard_backlight_timeout_note_manual_set(daemon_runtime_state* runtime, const int percent)
{
    runtime->keyboard_backlight_timed_off = false;
    runtime->keyboard_backlight_restore_percent = percent > 0 ? percent : 0;
}

/**
 * Advance keyboard backlight timeout state.
 *
 * The timeout turns the keyboard off after idle time and restores the last
 * non-zero brightness once input activity resumes.
 */
void keyboard_backlight_timeout_tick(
    struct ec_device* ec,
    const struct ans_config* cfg,
    daemon_runtime_state* runtime,
    const int64_t now_ms,
    const int64_t last_activity_ms
)
{
    struct keyboard_backlight_status status;
    const int64_t idle_ms = now_ms - last_activity_ms;
    const int timeout_ms = runtime->keyboard_backlight_timeout_seconds * 1000;

    if (!keyboard_backlight_timeout_can_run(cfg, runtime)) return;

    keyboard_backlight_update_restore_percent(ec, cfg, runtime, &status);

    if (keyboard_backlight_should_turn_off(runtime, idle_ms, timeout_ms))
    {
        if (keyboard_backlight_set_percent(ec, cfg, 0, &status))
        {
            runtime->keyboard_backlight_timed_off = true;
            keyboard_backlight_log_timed_off(idle_ms, runtime->keyboard_backlight_restore_percent);
        }

        return;
    }

    if (keyboard_backlight_should_restore(runtime, idle_ms, timeout_ms))
    {
        if (keyboard_backlight_set_percent(ec, cfg, runtime->keyboard_backlight_restore_percent, &status))
        {
            runtime->keyboard_backlight_timed_off = false;

            keyboard_backlight_log_restored(&status);
        }
    }
}
