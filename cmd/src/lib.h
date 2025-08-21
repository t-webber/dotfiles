#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

typedef const char *const *const Args;

#define NEW_STRING(str) {str, sizeof(str) - 1}

typedef struct {
        const char *const value;
        const size_t length;
} String;

String get_env_subpath(const String subpath, const char *const var);

#define panic(...)                                                             \
        {                                                                      \
                fprintf(stderr, __VA_ARGS__);                                  \
                exit(1);                                                       \
        }

bool is_verbose(const char *const program_name, const char *const normal_name,
                const char *const verbose_name);

const char *argv_one_filename(const int argc, const char *const *const argv);

bool has_slash(const char *const arg, size_t *len, const char **const position);

void store_usage(const char *const *const argv);

size_t utf8_strlen(const char *s);

static void exv(Args args) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
        execvp(args[0], (char *const *const)args);
        panic("Failed to execute %s\n", args[0]);
#pragma GCC diagnostic pop
}

#define exl(cmd, ...)                                                          \
        {                                                                      \
                execlp(cmd, __VA_ARGS__, NULL);                                \
                panic("Failed to execute %s.\n", cmd)                          \
        }

pid_t fork_checked(void);
void fork_wait(pid_t pid);
