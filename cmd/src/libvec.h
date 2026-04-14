#pragma once

#include "lib.h"
#include <stddef.h>

#define make_vec_header(name, suffix, ty)                                      \
                                                                               \
        typedef struct {                                                       \
                ty *data;                                                      \
                size_t len;                                                    \
                size_t cap;                                                    \
        } name;                                                                \
                                                                               \
        __wur name new_##suffix(void);                                         \
                                                                               \
        __nonnull((1)) void push_##suffix(name *const vec, ty val);            \
                                                                               \
        __nonnull() __wur ty pop_##suffix(name *const vec);                    \
                                                                               \
        __nonnull() void reserve_##suffix(name *const vec,                     \
                                          const size_t additional);            \
                                                                               \
        __nonnull() void extend_##suffix(name *const vec,                      \
                                         ty *other,                            \
                                         const size_t len)

make_vec_header(Vec, v, const char *);
make_vec_header(String, s, char);
