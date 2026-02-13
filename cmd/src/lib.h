#pragma once

#define _GNU_SOURCE

#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h> // IWYU pragma: keep
#include <string.h> // IWYU pragma: keep
#include <sys/types.h>

#define A(x) __attribute__((x))

#undef __wur
#define __wur A(__warn_unused_result__)

#undef _Noreturn
#define _Noreturn A(__noreturn__)

typedef char *var_str;
typedef const char *const_var_str;
typedef char *const str;
typedef const char *const const_str;
typedef const_str *const Args;

#define NEW_STRING(str) {str, sizeof(str) - 1}

typedef struct {
        const_str value;
        const size_t length;
} String;

#ifdef __nonnull
#undef __nonnull
#endif
#define __nonnull(params) A(__nonnull__ params)

#ifdef __attribute_pure__
#undef __attribute_pure__
#endif
#define __attribute_pure__ A(__pure__)

#ifdef __attribute_const__
#undef __attribute_const__
#endif
#define __attribute_const__ A(__const__)

#ifdef __attribute_malloc__
#undef __attribute_malloc__
#endif
#define __attribute_malloc__ A(__malloc__)

__wur __nonnull() String get_env_subpath(const String subpath, const_str var);

#define starts_with_const(var, rodata)                                         \
        (!strncmp(var, rodata, sizeof(rodata) - 1))

#define epanic(...)                                                            \
        {                                                                      \
                char msg[1024];                                                \
                char *wtr = stpcpy(msg, RED);                                  \
                int offset = sprintf(wtr, __VA_ARGS__);                        \
                stpcpy(wtr + offset, RESET);                                   \
                perror(msg);                                                   \
                exit(6);                                                       \
        }

#define upanic(...)                                                            \
        {                                                                      \
                char msg[1024];                                                \
                char *wtr = stpcpy(msg, RED);                                  \
                int offset = sprintf(wtr, __VA_ARGS__);                        \
                stpcpy(wtr + offset, RESET "\n");                              \
                fprintf(stderr, "%s", msg);                                    \
                fflush(stderr);                                                \
                exit(6);                                                       \
        }

__wur __nonnull() bool is_verbose(const_str program_name,
                                  const_str normal_name,
                                  const_str verbose_name);

__wur __nonnull() const
    char *argv_one_filename(const int argc, const_str *const argv);

__attribute_pure__ __wur
__nonnull() bool has_slash(const_str arg,
                           size_t *len,
                           const char **const position);

__nonnull() void store_usage(const_str prog_name,
                             const_str arg,
                             const bool is_alias);

__attribute_pure__ __wur __nonnull() size_t utf8_strlen(const char *s);

__nonnull() __wur const_var_str getenv_checked(const_str var);
__nonnull() void setenv_checked(const_str var, const_str val);

typedef enum {
        BATTERY_STATUS_CHARGING,
        BATTERY_STATUS_DISCHARGING,
        BATTERY_STATUS_FULL,
        BATTERY_STATUS_UNKNOWN
} battery_status;

__wur var_str get_battery_level(void);
__wur battery_status get_battery_status(void);

__attribute_const__ __wur size_t max(const size_t a, const size_t b);

#define array_len(x) sizeof(x) / sizeof(x[0])

__nonnull() void print_inline_array(const char *const *const array);
void print_inline_variadic(const_str first, ...);

__wur __nonnull() bool is_file_binary(const_str path);

#include "libterm.h" // IWYU pragma: keep
