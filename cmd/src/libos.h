#pragma once

#include "lib.h"
#include "libvec.h"
#include <dirent.h>

bool __wur __nonnull() is_dir(const_str path);
__nonnull() void chdir_checked(const_str path);
__nonnull() __wur bool is_file(const_str path);
__nonnull() void append_file(const_str path, const_str data);
__nonnull() void chmod_checked(const_str path, const mode_t mode);
__nonnull() __wur FILE *fopen_checked(const_str file_name, const_str mode);
__nonnull() __wur FILE *popen_checked(const_str command);
__nonnull() __wur int open_checked(const_str file_name, const int mode);
__nonnull() __wur DIR *opendir_checked(const_str file_name);
__nonnull() void closedir_checked(DIR *dirp);
__nonnull() void touch_checked(const_str filename);
__nonnull() void write_all(FILE *file, const_str buf, const size_t len);
__nonnull() void close_checked(int fd);
__nonnull() void fclose_checked(FILE *const fd);
__nonnull() __wur Vec alphascan_dir(const_str folder);
