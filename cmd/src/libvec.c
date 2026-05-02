#include "libvec.h"
#include "lib.h"
#include <assert.h>
#include <stdlib.h>

#define make_vec_body(name, suffix, ty, default_cap, empty)                    \
                                                                               \
        static const size_t DEFAULT_CAP_##suffix = default_cap;                \
        static const size_t PTR_SIZE_##suffix = sizeof(ty);                    \
                                                                               \
        __wur name new_##suffix(void) {                                        \
                ty *data = malloc(DEFAULT_CAP_##suffix * PTR_SIZE_##suffix);   \
                if (!data) epanic("failed to malloc");                         \
                                                                               \
                return (name){.data = data,                                    \
                              .cap = DEFAULT_CAP_##suffix,                     \
                              .len = 0};                                       \
        }                                                                      \
                                                                               \
        __nonnull((1)) void push_##suffix(name *const vec, ty val) {           \
                if (vec->len == vec->cap) {                                    \
                        vec->cap <<= 1;                                        \
                        vec->data = realloc(vec->data,                         \
                                            vec->cap * PTR_SIZE_##suffix);     \
                        if (!vec->data) epanic("failed to realloc");           \
                }                                                              \
                assert(vec->len < vec->cap);                                   \
                                                                               \
                vec->data[vec->len++] = val;                                   \
        }                                                                      \
                                                                               \
        __nonnull() __wur ty pop_##suffix(name *const vec) {                   \
                if (vec->len == 0) return empty;                               \
                return vec->data[--vec->len];                                  \
        }                                                                      \
                                                                               \
        __nonnull() void reserve_##suffix(name *const vec,                     \
                                          const size_t additional) {           \
                const size_t min_cap = vec->len + additional;                  \
                                                                               \
                if (min_cap <= vec->cap) return;                               \
                                                                               \
                const size_t new_cap = max(min_cap, vec->cap << 1);            \
                vec->cap = new_cap;                                            \
                vec->data = realloc(vec->data, vec->cap * PTR_SIZE_##suffix);  \
                if (!vec->data) epanic("failed to realloc");                   \
        }                                                                      \
                                                                               \
        __nonnull() void extend_##suffix(name *const vec,                      \
                                         ty const *other,                      \
                                         const size_t len) {                   \
                reserve_##suffix(vec, len);                                    \
                for (size_t i = 0; i < len; ++i) {                             \
                        push_##suffix(vec, other[i]);                          \
                }                                                              \
        }                                                                      \
                                                                               \
        __wur ty last_##suffix(name *const vec) {                              \
                if (vec->len == 0) return empty;                               \
                return vec->data[vec->len - 1];                                \
        }

// clang-format off
make_vec_body(Vec, v, const char *, 8, NULL)
make_vec_body(String, s, char, 64, '\0')
