#pragma once

#include "lib.h"
#include <stdbool.h>
#include <stddef.h>

__attribute_pure__ __wur __nonnull() const
    char *get_filename_extension(const_str filename, const size_t len);

_Noreturn __nonnull() void exec_open_file(const_str filename,
                                          const_str extension,
                                          const bool is_open,
                                          const bool is_kitty,
                                          const bool is_verbose);
