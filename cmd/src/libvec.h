#pragma once

#include "lib.h"
#include <stddef.h>

typedef struct {
        const char **data;
        size_t len;
        size_t cap;
} Vec;

__wur Vec new_vec(void);

__nonnull((1)) void push(Vec *const vec, const_str val);

__nonnull() __wur const_var_str pop(Vec *const vec);

__nonnull() void reserve(Vec *const vec, const size_t additional);

__nonnull() void extend(Vec *const vec, Args other, const size_t len);
