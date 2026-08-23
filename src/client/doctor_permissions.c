#include "client/doctor_util.h"

#include "core/constants.h"

#include <grp.h>
#include <pwd.h>
#include <errno.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

/**
 * Print model.
 *
 * Doctor output is used for support and hardware discovery, so each section
 * favors explicit facts over inferred state.
 */
void doctor_print_model_config(void)
{
    struct stat st;
    char target[PATH_MAX];
    char resolved[PATH_MAX];
    const char* path = ANS_DEFAULT_CONFIG;

    printf("$ inspect %s\n", path);

    if (lstat(path, &st) < 0)
    {
        printf("config=unavailable path=%s error=%s\n", path, strerror(errno));

        return;
    }

    printf("config=%s mode=%04o uid=%ld gid=%ld", path, (unsigned int)(st.st_mode & 07777), (long)st.st_uid, (long)st.st_gid);

    if (S_ISLNK(st.st_mode))
    {
        const ssize_t len = readlink(path, target, sizeof(target) - 1);

        if (len >= 0)
        {
            target[len] = '\0';

            printf(" target=%s", target);
        }
    }

    putchar('\n');

    if (realpath(path, resolved))
    {
        printf("%s\n", resolved);
    }
    else
    {
        printf("config_realpath=unavailable path=%s error=%s\n", path, strerror(errno));
    }
}

/**
 * Print socket permissions.
 *
 * Doctor output is used for support and hardware discovery, so each section
 * favors explicit facts over inferred state.
 */
void doctor_print_socket_permissions(void)
{
    struct stat st;

    if (stat(ANS_SOCKET_PATH, &st) < 0)
    {
        printf("socket=unavailable path=%s error=%s\n", ANS_SOCKET_PATH, strerror(errno));

        return;
    }

    const struct group* group = getgrgid(st.st_gid);

    printf(
        "socket=%s mode=%04o uid=%ld gid=%ld group=%s\n",
        ANS_SOCKET_PATH,
        (unsigned int)(st.st_mode & 07777),
        (long)st.st_uid,
        (long)st.st_gid,
        group ? group->gr_name : "unknown"
    );
}

/**
 * Print user groups.
 *
 * Doctor output is used for support and hardware discovery, so each section
 * favors explicit facts over inferred state.
 */
void doctor_print_user_groups(void)
{
    const struct passwd* passwd = getpwuid(getuid());
    const int group_size = getgroups(0, NULL);

    gid_t* groups = NULL;

    printf("uid=%ld user=%s gid=%ld\n", (long)getuid(), passwd ? passwd->pw_name : "unknown", (long)getgid());

    if (group_size <= 0) return;

    groups = calloc((size_t)group_size, sizeof(*groups));

    if (!groups) return;

    if (getgroups(group_size, groups) == group_size)
    {
        printf("groups=");

        for (int i = 0; i < group_size; i++)
        {
            const struct group* group = getgrgid(groups[i]);

            printf("%s%ld%s%s", i == 0 ? "" : ",", (long)groups[i], group ? ":" : "", group ? group->gr_name : "");
        }

        putchar('\n');
    }

    free(groups);
}
