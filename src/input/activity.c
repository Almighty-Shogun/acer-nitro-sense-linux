#include "input/activity.h"

#include <fcntl.h>
#include <stdio.h>
#include <dirent.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <linux/input.h>

#define BIT_WORD(nr) ((nr) / (8 * (int)sizeof(unsigned long)))
#define BIT_MASK(nr) (1UL << ((nr) % (8 * (int)sizeof(unsigned long))))

/**
 * Return whether an evdev capability bit is set.
 *
 * The Linux input API exposes supported event types as bit fields. This helper
 * keeps the word and mask calculation in one place.
 */
static bool bit_is_set(const unsigned long* bits, const int bit)
{
    return (bits[BIT_WORD(bit)] & BIT_MASK(bit)) != 0;
}

/**
 * Return whether an input device reports key events.
 *
 * Keyboard-backlight timeout only needs activity from devices that emit key
 * events, so pointer-only or switch-only devices are skipped.
 */
static bool input_fd_has_key_events(const int fd)
{
    unsigned long ev_bits[BIT_WORD(EV_MAX) + 1] = {0};

    if (ioctl(fd, EVIOCGBIT(0, sizeof(ev_bits)), ev_bits) < 0)
        return false;

    return bit_is_set(ev_bits, EV_KEY);
}

/**
 * Initialize the input activity monitor.
 *
 * All descriptors start closed and the last-activity timestamp is seeded with
 * the current monotonic time so startup is not treated as idle.
 */
void input_activity_init(input_activity_monitor* monitor, const int64_t now_ms)
{
    memset(monitor, 0, sizeof(*monitor));

    monitor->last_activity_ms = now_ms;

    for (int i = 0; i < ANS_INPUT_ACTIVITY_MAX_FDS; i++)
        monitor->fds[i] = -1;
}

/**
 * Close every input descriptor owned by the monitor.
 *
 * Rebuilding the watched device list uses the same cleanup path as daemon
 * shutdown, so descriptors are always reset after close.
 */
void input_activity_close(input_activity_monitor* monitor)
{
    for (int i = 0; i < monitor->fd_len; i++)
    {
        if (monitor->fds[i] >= 0)
            close(monitor->fds[i]);

        monitor->fds[i] = -1;
    }
    monitor->fd_len = 0;
}

/**
 * Open input event devices that can report keyboard activity.
 *
 * Permission failures are ignored per-device because the daemon can still
 * manage fans and keyboard lighting without activity monitoring.
 */
void input_activity_open(input_activity_monitor* monitor, const int64_t now_ms)
{
    struct dirent* entry;

    input_activity_close(monitor);
    monitor->last_activity_ms = now_ms;

    DIR* dir = opendir("/dev/input");

    if (!dir) return;

    while (((entry = readdir(dir))) && monitor->fd_len < ANS_INPUT_ACTIVITY_MAX_FDS)
    {
        char path[PATH_MAX];

        if (strncmp(entry->d_name, "event", 5) != 0) continue;
        if (strlen(entry->d_name) > sizeof(path) - sizeof("/dev/input/")) continue;

        snprintf(path, sizeof(path), "/dev/input/%s", entry->d_name);

        const int fd = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC);

        if (fd < 0) continue;

        if (!input_fd_has_key_events(fd))
        {
            close(fd);

            continue;
        }

        monitor->fds[monitor->fd_len++] = fd;
    }

    closedir(dir);
}

/**
 * Add input devices to the poll set.
 *
 * The daemon owns the select() loop. This helper only registers the monitor's
 * readable descriptors and raises the maximum descriptor when needed.
 */
void input_activity_add_fds(const input_activity_monitor* monitor, fd_set* readfds, int* max_fd)
{
    for (int i = 0; i < monitor->fd_len; i++)
    {
        if (monitor->fds[i] < 0) continue;

        FD_SET(monitor->fds[i], readfds);

        if (monitor->fds[i] > *max_fd)
            *max_fd = monitor->fds[i];
    }
}

/**
 * Process ready input events.
 *
 * Any pressed key marks the system active. The key code is intentionally
 * ignored because the timeout feature only needs an idle/active signal.
 */
void input_activity_handle_ready(input_activity_monitor* monitor, const fd_set* readfds, const int64_t now_ms)
{
    for (int i = 0; i < monitor->fd_len; i++)
    {
        bool active = false;
        struct input_event ev;

        ssize_t n;

        if (monitor->fds[i] < 0 || !FD_ISSET(monitor->fds[i], readfds)) continue;

        while ((n = read(monitor->fds[i], &ev, sizeof(ev))) == sizeof(ev))
        {
            if (ev.type == EV_KEY && ev.value != 0)
                active = true;
        }

        if (active)
            monitor->last_activity_ms = now_ms;
    }
}
