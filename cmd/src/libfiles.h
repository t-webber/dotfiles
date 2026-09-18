#pragma once

#include "lib.h"
#include <stdbool.h>
#include <stddef.h>

__attribute_pure__ __wur __nonnull() const
    char *get_filename_extension(const_str filename);

typedef enum { DISPLAY_OPEN, DISPLAY_VERBOSE, DISPLAY_RAW } display_type;

_Noreturn __nonnull() void exec_open_file(const_str filename,
                                          const display_type ty);
