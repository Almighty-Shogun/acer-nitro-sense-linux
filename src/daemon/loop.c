#include "daemon/loop.h"

#include "fan/control.h"
#include "daemon/status.h"
#include "input/activity.h"
#include "platform/power_source.h"
#include "commands/daemon/daemon.h"
#include "keyboard/backlight_timeout.h"

#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/socket.h>

/**
 * Read the monotonic clock in milliseconds.
 *
 * The daemon loop coordinates polling, safety checks, and socket handling.
 * Keeping this path predictable prevents fan control from depending on UI
 * timing.
 */
static int64_t monotonic_ms(void)
{
    struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) < 0)
        return 0;

    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

/**
 * Convert an absolute deadline into a poll timeout.
 *
 * The daemon loop coordinates polling, safety checks, and socket handling.
 * Keeping this path predictable prevents fan control from depending on UI
 * timing.
 */
static struct timeval timeout_from_ms(int64_t ms)
{
    struct timeval tv;

    if (ms < 0)
        ms = 0;

    tv.tv_usec = ms % 1000 * 1000;
    tv.tv_sec = (time_t)(ms / 1000);

    return tv;
}

/**
 * Apply apply.
 *
 * The daemon loop coordinates polling, safety checks, and socket handling.
 * Keeping this path predictable prevents fan control from depending on UI
 * timing.
 */
static void maybe_apply_power_source_profile(
    struct ec_device* ec,
    const struct ans_config* cfg,
    enum power_source_state* last_source,
    const daemon_runtime_state* runtime
)
{
    const enum power_source_state source = read_power_source();

    if (source == POWER_SOURCE_UNKNOWN || source == *last_source) return;

    *last_source = source;

    if (!runtime || !runtime->power_source_auto_apply) return;

    const char* profile = power_source_profile_for(cfg, source);

    if (!profile) return;

    if (apply_power_source_profile(ec, cfg, source))
    {
        if (!daemon_quiet_logs)
            fprintf(stderr, "power_source_profile_apply source=%s profile=%s\n", power_source_name(source), profile);
    }
    else
    {
        fprintf(stderr, "warning: failed to apply power source profile source=%s profile=%s\n", power_source_name(source), profile);
    }
}

/**
 * Write runtime.
 *
 * The daemon loop coordinates polling, safety checks, and socket handling.
 * Keeping this path predictable prevents fan control from depending on UI
 * timing.
 */
static void write_runtime_status(
    struct ec_device* ec, const struct ans_config* cfg,
    const fan_state states[ANS_MAX_FANS],
    const bool auto_mode, const char* preset,
    const bool coolboost_enabled,
    const hardware_names* names,
    const daemon_runtime_state* runtime
)
{
    write_status(cfg, ec, states, auto_mode, preset, coolboost_enabled, names, runtime);
    write_temperature_cache(cfg, states);
}

/**
 * Run one daemon control-loop iteration.
 *
 * The daemon loop coordinates polling, safety checks, and socket handling.
 * Keeping this path predictable prevents fan control from depending on UI
 * timing.
 */
void run_daemon_loop(
    const int sock_fd,
    struct ec_device* ec,
    const struct ans_config* cfg,
    fan_state states[ANS_MAX_FANS],
    bool* auto_mode,
    char* preset,
    const size_t preset_len,
    bool* coolboost_enabled,
    const hardware_names* names,
    daemon_runtime_state* runtime
)
{
    int64_t next_update_ms = 0;
    input_activity_monitor input_monitor;

    enum power_source_state last_power_source = POWER_SOURCE_UNKNOWN;

    input_activity_init(&input_monitor, monotonic_ms());

    if (cfg->keyboard_backlight.timeout_supported)
        input_activity_open(&input_monitor, monotonic_ms());

    while (daemon_running)
    {
        fd_set readfds;
        struct timeval tv;
        const int64_t now_ms = monotonic_ms();

        if (next_update_ms <= now_ms)
        {
            maybe_apply_power_source_profile(ec, cfg, &last_power_source, runtime);

            if (input_monitor.fd_len > 0)
                keyboard_backlight_timeout_tick(ec, cfg, runtime, now_ms, input_monitor.last_activity_ms);

            update_fan_states(ec, cfg, states, *auto_mode, preset);
            write_runtime_status(ec, cfg, states, *auto_mode, preset, *coolboost_enabled, names, runtime);

            next_update_ms = monotonic_ms() + cfg->poll_interval_ms;
        }

        FD_ZERO(&readfds);
        FD_SET(sock_fd, &readfds);

        int max_fd = sock_fd;

        input_activity_add_fds(&input_monitor, &readfds, &max_fd);

        tv = timeout_from_ms(next_update_ms - monotonic_ms());

        const int ready = select(max_fd + 1, &readfds, NULL, NULL, &tv);

        if (ready > 0)
        {
            input_activity_handle_ready(&input_monitor, &readfds, monotonic_ms());

            if (input_monitor.fd_len > 0)
                keyboard_backlight_timeout_tick(ec, cfg, runtime, monotonic_ms(), input_monitor.last_activity_ms);

            if (FD_ISSET(sock_fd, &readfds))
            {
                const int client = accept4(sock_fd, NULL, NULL, SOCK_CLOEXEC);

                if (client >= 0)
                {
                    handle_client(client, ec, cfg, states, auto_mode, preset, preset_len, coolboost_enabled, runtime);

                    close(client);

                    write_runtime_status(ec, cfg, states, *auto_mode, preset, *coolboost_enabled, names, runtime);
                }
            }
        }
        else if (ready < 0 && errno != EINTR)
        {
            perror("select");

            break;
        }
    }

    input_activity_close(&input_monitor);
}
