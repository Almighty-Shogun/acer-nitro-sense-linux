#include "control/socket.h"

#include "core/constants.h"
#include "util/file.h"

#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

static int bind_control_socket(const int fd)
{
    struct sockaddr_un addr;

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", ANS_SOCKET_PATH);

    return bind(fd, (struct sockaddr *)&addr, sizeof(addr));
}

static void assign_socket_group(void)
{
    const struct group *group = getgrnam(ANS_CONTROL_GROUP);

    if (!group) {
        fprintf(stderr, "warning: group %s does not exist; control socket requires root\n",
                ANS_CONTROL_GROUP);
        return;
    }

    struct stat st;

    if (stat(ANS_SOCKET_PATH, &st) == 0 && st.st_gid != group->gr_gid &&
        chown(ANS_SOCKET_PATH, (uid_t)-1, group->gr_gid) < 0)
        fprintf(stderr,
                "warning: failed to set control socket group to %s: %s\n",
                ANS_CONTROL_GROUP, strerror(errno));
}

static int finish_control_socket(const int fd)
{
    if (chmod(ANS_SOCKET_PATH, 0660) < 0 || listen(fd, 8) < 0) {
        close(fd);
        unlink(ANS_SOCKET_PATH);

        return -1;
    }

    return fd;
}

int make_socket(void)
{
    if (mkdir_p(ANS_RUN_DIR) < 0)
        return -1;

    unlink(ANS_SOCKET_PATH);

    const int fd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);

    if (fd < 0)
        return -1;

    if (bind_control_socket(fd) < 0) {
        close(fd);
        unlink(ANS_SOCKET_PATH);

        return -1;
    }

    assign_socket_group();
    return finish_control_socket(fd);
}

static bool groups_line_has_group(const char *line, const gid_t group)
{
    const char *p = strchr(line, ':');

    if (!p)
        return false;

    p++;
    while (*p && *p != '\n') {
        char *end;
        const long value = strtol(p, &end, 10);

        if (p == end) {
            p++;
            continue;
        }

        if ((gid_t)value == group)
            return true;

        p = end;
    }

    return false;
}

static bool pid_has_group(const pid_t pid, const gid_t group)
{
    char path[64];
    bool allowed = false;

    snprintf(path, sizeof(path), "/proc/%ld/status", (long)pid);

    char *status = read_text_file(path, 64 * 1024);

    if (!status)
        return false;

    char *line = strstr(status, "\nGroups:");

    if (!line && strncmp(status, "Groups:", 7) == 0)
        line = status;

    if (line)
        allowed = groups_line_has_group(line, group);

    free(status);

    return allowed;
}

static bool uid_is_group_member(const uid_t uid, const gid_t group_gid)
{
    struct passwd *passwd = getpwuid(uid);
    struct group *group = getgrgid(group_gid);

    if (!passwd || !group)
        return false;

    if (passwd->pw_gid == group_gid)
        return true;

    for (char **member = group->gr_mem; member && *member; member++) {
        if (strcmp(*member, passwd->pw_name) == 0)
            return true;
    }

    return false;
}

bool client_can_control(const int client)
{
    struct ucred cred;
    socklen_t len = sizeof(cred);

    if (getsockopt(client, SOL_SOCKET, SO_PEERCRED, &cred, &len) < 0)
        return false;

    if (cred.uid == 0)
        return true;

    struct group *group = getgrnam(ANS_CONTROL_GROUP);

    if (!group)
        return false;

    return cred.gid == group->gr_gid ||
           pid_has_group(cred.pid, group->gr_gid) ||
           uid_is_group_member(cred.uid, group->gr_gid);
}
