#ifndef ANS_INPUT_ACTIVITY_H
#define ANS_INPUT_ACTIVITY_H

#include <stdint.h>
#include <sys/select.h>

#define ANS_INPUT_ACTIVITY_MAX_FDS 64

/**
 * Open input devices and last-seen activity timestamp.
 *
 * Keyboard backlight timeout uses this monitor to restore or dim lighting
 * based on user activity without depending on a desktop session.
 */
typedef struct
{
    int fds[ANS_INPUT_ACTIVITY_MAX_FDS];
    int fd_len;
    int64_t last_activity_ms;
} input_activity_monitor;

/**
 * Initialize an input activity monitor.
 *
 * The initial timestamp prevents the timeout code from treating startup as a
 * long idle period.
 */
void input_activity_init(input_activity_monitor* monitor, int64_t now_ms);

/**
 * Close all input descriptors owned by the monitor.
 *
 * The daemon calls this during shutdown and when rebuilding the watched input
 * set.
 */
void input_activity_close(input_activity_monitor* monitor);

/**
 * Open readable input event devices for activity tracking.
 *
 * Devices that cannot be opened are skipped so the daemon can still run on
 * systems with stricter input permissions.
 */
void input_activity_open(input_activity_monitor* monitor, int64_t now_ms);

/**
 * Add input devices to the poll set.
 *
 * The caller owns select() setup; this helper contributes the monitor's file
 * descriptors and updates the maximum descriptor.
 */
void input_activity_add_fds(const input_activity_monitor* monitor, fd_set* readfds, int* max_fd);

/**
 * Update the last activity timestamp for readable input descriptors.
 *
 * Any readable event counts as user activity; the event payload itself is not
 * interpreted.
 */
void input_activity_handle_ready(input_activity_monitor* monitor, const fd_set* readfds, int64_t now_ms);

#endif
