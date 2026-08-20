#include "input/activity.h"

#include <dirent.h>
#include <fcntl.h>
#include <linux/input.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define BIT_WORD(nr) ((nr) / (8 * (int)sizeof(unsigned long)))
#define BIT_MASK(nr) (1UL << ((nr) % (8 * (int)sizeof(unsigned long))))

static bool bit_is_set(const unsigned long *bits, const int bit)
{
    return (bits[BIT_WORD(bit)] & BIT_MASK(bit)) != 0;
}

static bool input_fd_has_key_events(const int fd)
{
    unsigned long ev_bits[BIT_WORD(EV_MAX) + 1];

    memset(ev_bits, 0, sizeof(ev_bits));
    if (ioctl(fd, EVIOCGBIT(0, sizeof(ev_bits)), ev_bits) < 0)
        return false;

    return bit_is_set(ev_bits, EV_KEY);
}

void input_activity_init(input_activity_monitor *monitor, const int64_t now_ms)
{
    memset(monitor, 0, sizeof(*monitor));
    monitor->last_activity_ms = now_ms;
    for (int i = 0; i < ANS_INPUT_ACTIVITY_MAX_FDS; i++)
        monitor->fds[i] = -1;
}

void input_activity_close(input_activity_monitor *monitor)
{
    for (int i = 0; i < monitor->fd_len; i++) {
        if (monitor->fds[i] >= 0)
            close(monitor->fds[i]);
        monitor->fds[i] = -1;
    }
    monitor->fd_len = 0;
}

void input_activity_open(input_activity_monitor *monitor, const int64_t now_ms)
{
    DIR *dir;
    struct dirent *entry;

    input_activity_close(monitor);
    monitor->last_activity_ms = now_ms;

    dir = opendir("/dev/input");
    if (!dir)
        return;

    while ((entry = readdir(dir)) &&
           monitor->fd_len < ANS_INPUT_ACTIVITY_MAX_FDS) {
        char path[PATH_MAX];
        int fd;

        if (strncmp(entry->d_name, "event", 5) != 0)
            continue;
        if (strlen(entry->d_name) > sizeof(path) - sizeof("/dev/input/"))
            continue;

        snprintf(path, sizeof(path), "/dev/input/%s", entry->d_name);
        fd = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
        if (fd < 0)
            continue;

        if (!input_fd_has_key_events(fd)) {
            close(fd);
            continue;
        }

        monitor->fds[monitor->fd_len++] = fd;
    }

    closedir(dir);
}

void input_activity_add_fds(const input_activity_monitor *monitor,
                            fd_set *readfds, int *max_fd)
{
    for (int i = 0; i < monitor->fd_len; i++) {
        if (monitor->fds[i] < 0)
            continue;

        FD_SET(monitor->fds[i], readfds);
        if (monitor->fds[i] > *max_fd)
            *max_fd = monitor->fds[i];
    }
}

void input_activity_handle_ready(input_activity_monitor *monitor,
                                 const fd_set *readfds, const int64_t now_ms)
{
    for (int i = 0; i < monitor->fd_len; i++) {
        bool active = false;
        struct input_event ev;
        ssize_t n;

        if (monitor->fds[i] < 0 || !FD_ISSET(monitor->fds[i], readfds))
            continue;

        while ((n = read(monitor->fds[i], &ev, sizeof(ev))) == sizeof(ev)) {
            if (ev.type == EV_KEY && ev.value != 0)
                active = true;
        }

        if (active)
            monitor->last_activity_ms = now_ms;
    }
}
