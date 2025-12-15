#if !defined(_WIN32)

#include "platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

// Get ~/Documents
int platform_get_documents_dir(char *out, size_t out_sz) {
    const char *home = getenv("HOME");
    if (!home) return -1;

    if (snprintf(out, out_sz, "%s/Documents", home) >= (int)out_sz) {
        return -1;
    }
    return 0;
}

// mkdir -p
int platform_mkdir_p(const char *path) {
    char tmp[1024];
    size_t len;

    if (!path) return -1;

    strncpy(tmp, path, sizeof(tmp));
    tmp[sizeof(tmp) - 1] = '\0';
    len = strlen(tmp);

    if (len == 0) return -1;
    if (tmp[len - 1] == '/') tmp[len - 1] = '\0';

    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
                return -1;
            }
            *p = '/';
        }
    }

    if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
        return -1;
    }

    return 0;
}

#endif
