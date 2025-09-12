#pragma once

#define _GNU_SOURCE

#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef char * var_str;
typedef const char * const_var_str;
typedef char *const str;
typedef const char *const const_str;
typedef const_str *const Args;

#define NEW_STRING(str) {str, sizeof(str) - 1}

typedef struct {
        const_str value;
        const size_t length;
} String;

String get_env_subpath(const String subpath, const_str var);

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

void panic_err(const_str message);

bool is_verbose(const_str program_name,
                const_str normal_name,
                const_str verbose_name);

const char *argv_one_filename(const int argc, const_str *const argv);

bool has_slash(const_str arg, size_t *len, const char **const position);

void store_usage(const_str *const argv);

size_t utf8_strlen(const char *s);

FILE *fopen_checked(const_str file_name, const_str mode);
FILE *popen_checked(const_str command);
DIR *opendir_checked(const_str file_name);
const_var_str getenv_checked(const_str var);

void exl_notif(const_str message);

#define exl_corenotif(...)                                                     \
        char message[128];                                                     \
        sprintf(message, __VA_ARGS__);                                         \
        exl_notif(message);

void exl_err_notif(void);

var_str get_battery_level(void);
var_str get_battery_status(void);

#define max(a, b) a >= b ? a : b

void clear(void);
void exl_err_notif_msg(const_str msg);

#define A(x) __attribute__((x))
#define MUST_USE A(warn_unused_result)
#define NON_NULL A(nonnull)
#define NO_RETURN A(noreturn)

#define null_ch '\0'
