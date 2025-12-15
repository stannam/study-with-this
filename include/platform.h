#ifndef PLATFORM_H
#define PLATFORM_H

#include <stddef.h>

// Per platform (platform_posix or platform_win).
// Writes the path to the user's Documents directory into `out`.
// Return 0 on success.
int platform_get_documents_dir(char *out, size_t out_sz);

// Per platform (platform_posix or platform_win).
// Make sure `path` exists. Create intermediate directories if needed.
// Return 0 on success.
int platform_mkdir_p(const char *path);

// Returns the platform-specific path separator ('/' on POSIX, '\\' on Windows).
#if defined(_WIN32)
#define PLATFORM_PATH_SEP '\\'
#else
#define PLATFORM_PATH_SEP '/'
#endif

#endif
