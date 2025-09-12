#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef char *const str;
typedef const char *const const_str;
typedef const_str *const Args;

#define NEW_STRING(str) {str, sizeof(str) - 1}

typedef struct {
        const_str value;
        const size_t length;
} String;

String get_env_subpath(const String subpath, const_str var);

#define panic(...)                                                             \
        {                                                                      \
                char msg[256];                                                 \
                snprintf(msg, sizeof(msg), __VA_ARGS__);                       \
                perror(msg);                                                   \
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
const_str getenv_checked(const_str var);

void exl_notif(const_str message);

#define exl_corenotif(...)                                                     \
        char message[128];                                                     \
        sprintf(message, __VA_ARGS__);                                         \
        exl_notif(message);

void exl_err_notif(void);

char *get_battery_level(void);
char *get_battery_status(void);
