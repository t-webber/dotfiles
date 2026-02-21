#pragma once

#include "lib.h"
#include <dirent.h>

bool __wur __nonnull() is_dir(const_str path);
__nonnull() void chdir_checked(const_str path);
__nonnull() __wur bool is_file(const_str path);
__nonnull() void append_file(const_str path, const_str data);
__nonnull() void chmod_checked(const_str path, const mode_t mode);
__wur __nonnull() FILE *fopen_checked(const_str file_name, const_str mode);
__wur __nonnull() FILE *popen_checked(const_str command);
__nonnull() __wur DIR *opendir_checked(const_str file_name);
__nonnull() void closedir_checked(DIR *dirp);
__nonnull() void touch_checked(const_str filename);
