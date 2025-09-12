#pragma once

#include "lib.h"
#include <stddef.h>

typedef struct {
        const char **data;
        size_t len;
        size_t cap;
} Vec;

Vec new_vec(void);

void push(Vec *const vec, const_str val);

void reserve(Vec *const vec, const size_t additional);
