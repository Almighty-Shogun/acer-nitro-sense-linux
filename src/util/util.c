#include "ans.h"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int mkdir_p(const char *path)
{
    char tmp[512];
    size_t len = strlen(path);

    if (len == 0 || len >= sizeof(tmp))
        return -1;

    memcpy(tmp, path, len + 1);
    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(tmp, 0755) < 0 && errno != EEXIST)
                return -1;
            *p = '/';
        }
    }

    if (mkdir(tmp, 0755) < 0 && errno != EEXIST)
        return -1;
    return 0;
}

char *read_text_file(const char *path, size_t limit)
{
    FILE *f = fopen(path, "re");
    char *buf;
    size_t len;

    if (!f)
        return NULL;

    buf = calloc(1, limit + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }

    len = fread(buf, 1, limit, f);
    buf[len] = '\0';
    fclose(f);
    return buf;
}

int write_text_file_atomic(const char *path, const char *content)
{
    char tmp[512];
    int fd;
    size_t len = strlen(content);

    if (snprintf(tmp, sizeof(tmp), "%s.tmp", path) >= (int)sizeof(tmp))
        return -1;

    fd = open(tmp, O_CREAT | O_TRUNC | O_WRONLY | O_CLOEXEC, 0644);
    if (fd < 0)
        return -1;

    if (write(fd, content, len) != (ssize_t)len) {
        close(fd);
        unlink(tmp);
        return -1;
    }

    if (close(fd) < 0) {
        unlink(tmp);
        return -1;
    }

    return rename(tmp, path);
}

bool string_contains_case(const char *haystack, const char *needle)
{
    size_t nlen = strlen(needle);

    if (nlen == 0)
        return true;

    for (const char *h = haystack; *h; h++) {
        size_t i = 0;
        while (i < nlen && h[i] &&
               tolower((unsigned char)h[i]) == tolower((unsigned char)needle[i]))
            i++;
        if (i == nlen)
            return true;
    }
    return false;
}

void trim_ascii(char *s)
{
    const char *start = s;

    while (*start == ' ' || *start == '\t' || *start == '\n' || *start == '\r')
        start++;

    if (start != s)
        memmove(s, start, strlen(start) + 1);

    char *end = s + strlen(s);

    while (end > s && (end[-1] == ' ' || end[-1] == '\t' ||
                       end[-1] == '\n' || end[-1] == '\r'))
        *--end = '\0';
}

int clamp_int(int value, int min, int max)
{
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}
