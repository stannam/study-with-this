#if defined(_WIN32)

#include "platform.h"

#include <windows.h>
#include <shlobj.h>
#include <string.h>

// Get Documents folder
int platform_get_documents_dir(char *out, size_t out_sz) {
    PWSTR wide = NULL;

    if (SHGetKnownFolderPath(&FOLDERID_Documents, 0, NULL, &wide) != S_OK) {
        return -1;
    }

    int result = WideCharToMultiByte(
        CP_UTF8, 0, wide, -1, out, (int)out_sz, NULL, NULL
    );

    CoTaskMemFree(wide);
    return (result > 0) ? 0 : -1;
}

// mkdir -p equivalent
int platform_mkdir_p(const char *path) {
    char *tmp;
    char *p;

    if (!path) return -1;

    size_t path_len= strlen(path);
    if (path_len== 0) return -1;

    tmp = malloc(path_len+ 1);
    if (!tmp) return -1;

    memcpy(tmp, path, path_len+ 1);

    // UNC path: \\server\share\...
    if (path_len>= 2 && tmp[0] == '\\' && tmp[1] == '\\') {
        // Skip \\server\share\
        p = tmp + 2;

        /* skip server */
        while (*p && *p != '\\') p++;
        if (*p == '\\') p++;

        /* skip share */
        while (*p && *p != '\\') p++;
        if (*p == '\\') p++;

    } else if (path_len>= 3 && tmp[1] == ':' && (tmp[2] == '\\' || tmp[2] == '/')) {
        // Skip drive prefix like C:\
        p = tmp + 3;
        } else {
            p = tmp + 1;
        }

    for (; *p; p++) {
        if (*p == '\\' || *p == '/') {
            char saved = *p;
            *p = '\0';
            if (!CreateDirectoryA(tmp, NULL)) {
                DWORD err = GetLastError();
                if (err != ERROR_ALREADY_EXISTS) {
                    free(tmp);
                    return -1;
                }
            }
            *p = saved;
        }
    }

    // catch CreateDirectoryA() errors
    if (!CreateDirectoryA(tmp, NULL)) {
        DWORD err = GetLastError();
        if (err != ERROR_ALREADY_EXISTS) {
            free(tmp);
            return -1;
        }
    }
    free(tmp);
    return 0;
}

#endif
