#pragma once

#include "stddef.h"
#include "sys/cdefs.h"

__nonnull() void sort(const char **const keybinds,
                      const size_t start,
                      const size_t end);
