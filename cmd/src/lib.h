#pragma once

#define _GNU_SOURCE

#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define null_ch '\0'
#define A(x) __attribute__((x))
#define mustuse A(warn_unused_result)
#define nonnull A(nonnull)
#define noreturn A(noreturn)

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

mustuse nonnull String get_env_subpath(const String subpath, const_str var);

#define epanic(...)                                                            \
        {                                                                      \
                char msg[256];                                                 \
                snprintf(msg, sizeof(msg), __VA_ARGS__);                       \
                perror(msg);                                                   \
                exit(6);                                                       \
        }

#define upanic(...)                                                            \
        {                                                                      \
                fprintf(stderr, __VA_ARGS__);                                  \
                exit(6);                                                       \
        }

nonnull void panic_err(const_str message);

mustuse nonnull bool is_verbose(const_str program_name,
                                const_str normal_name,
                                const_str verbose_name);

mustuse nonnull const char *argv_one_filename(const int argc,
                                              const_str *const argv);

mustuse nonnull bool
has_slash(const_str arg, size_t *len, const char **const position);

nonnull void store_usage(const_str *const argv);

mustuse nonnull size_t utf8_strlen(const char *s);

mustuse nonnull FILE *fopen_checked(const_str file_name, const_str mode);
mustuse nonnull FILE *popen_checked(const_str command);
nonnull mustuse DIR *opendir_checked(const_str file_name);
nonnull mustuse const_var_str getenv_checked(const_str var);

nonnull void exl_notif(const_str message);

#define exl_corenotif(...)                                                     \
        char message[128];                                                     \
        sprintf(message, __VA_ARGS__);                                         \
        exl_notif(message);

void exl_err_notif(void);

mustuse var_str get_battery_level(void);
mustuse var_str get_battery_status(void);

void clear(void);
void exl_err_notif_msg(const_str msg);

mustuse size_t max(const size_t a, const size_t b);
