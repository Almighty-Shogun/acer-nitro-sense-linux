#include "client/doctor_util.h"

#include "core/constants.h"

#include <errno.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

void doctor_print_model_config(void)
{
    const char *path = ANS_DEFAULT_CONFIG;
    struct stat st;
    char target[PATH_MAX];
    char resolved[PATH_MAX];

    printf("$ inspect %s\n", path);

    if (lstat(path, &st) < 0) {
        printf("config=unavailable path=%s error=%s\n", path, strerror(errno));
        return;
    }

    printf("config=%s mode=%04o uid=%ld gid=%ld", path,
           (unsigned int)(st.st_mode & 07777), (long)st.st_uid,
           (long)st.st_gid);

    if (S_ISLNK(st.st_mode)) {
        const ssize_t len = readlink(path, target, sizeof(target) - 1);

        if (len >= 0) {
            target[len] = '\0';
            printf(" target=%s", target);
        }
    }
    putchar('\n');

    if (realpath(path, resolved))
        printf("%s\n", resolved);
    else
        printf("config_realpath=unavailable path=%s error=%s\n", path,
               strerror(errno));
}

void doctor_print_socket_permissions(void)
{
    struct stat st;
    struct group *group;

    if (stat(ANS_SOCKET_PATH, &st) < 0) {
        printf("socket=unavailable path=%s error=%s\n", ANS_SOCKET_PATH, strerror(errno));
        return;
    }

    group = getgrgid(st.st_gid);
    printf("socket=%s mode=%04o uid=%ld gid=%ld group=%s\n",
           ANS_SOCKET_PATH, (unsigned int)(st.st_mode & 07777),
           (long)st.st_uid, (long)st.st_gid, group ? group->gr_name : "unknown");
}

void doctor_print_user_groups(void)
{
    struct passwd *passwd = getpwuid(getuid());
    const int ngroups = getgroups(0, NULL);
    gid_t *groups = NULL;

    printf("uid=%ld user=%s gid=%ld\n", (long)getuid(),
           passwd ? passwd->pw_name : "unknown", (long)getgid());

    if (ngroups <= 0)
        return;

    groups = calloc((size_t)ngroups, sizeof(*groups));
    if (!groups)
        return;

    if (getgroups(ngroups, groups) == ngroups) {
        printf("groups=");
        for (int i = 0; i < ngroups; i++) {
            struct group *group = getgrgid(groups[i]);
            printf("%s%ld%s%s", i == 0 ? "" : ",", (long)groups[i],
                   group ? ":" : "", group ? group->gr_name : "");
        }
        putchar('\n');
    }

    free(groups);
}
