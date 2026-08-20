#ifndef ANS_INPUT_ACTIVITY_H
#define ANS_INPUT_ACTIVITY_H

#include <stdint.h>
#include <sys/select.h>

#define ANS_INPUT_ACTIVITY_MAX_FDS 64

typedef struct {
    int fds[ANS_INPUT_ACTIVITY_MAX_FDS];
    int fd_len;
    int64_t last_activity_ms;
} input_activity_monitor;

void input_activity_init(input_activity_monitor *monitor, int64_t now_ms);
void input_activity_close(input_activity_monitor *monitor);
void input_activity_open(input_activity_monitor *monitor, int64_t now_ms);
void input_activity_add_fds(const input_activity_monitor *monitor,
                            fd_set *readfds, int *max_fd);
void input_activity_handle_ready(input_activity_monitor *monitor,
                                 const fd_set *readfds, int64_t now_ms);

#endif
