#pragma once

#include "lib.h"
#include "libvec.h"
#include <stddef.h>

#define cmd(alias, expanded, ...)                                              \
        {                                                                      \
                alias, expanded, (const char *[]) {                            \
                        __VA_ARGS__ NULL                                       \
                }                                                              \
        }

typedef struct {
        const char *alias;
        const char *expanded;
        const char **options;
} Cmd;

typedef struct {
        const char origin;
        const char *const replace;
} Manual;

typedef struct {
        const Cmd *const cmd;
        const size_t cmd_len;
        const Manual *const manual;
        const size_t manual_len;
} CliSettings;

__nonnull() _Noreturn void run_cli(const size_t argc,
                                   Args argv,
                                   const CliSettings *const settings,
                                   Vec *const cmd);

__nonnull() _Noreturn void run_cli_single(const size_t argc,
                                          Args argv,
                                          const Cmd *const cmd);

#define make_settings(SETTINGS, COMMANDS, MANUAL)                              \
        static const CliSettings SETTINGS = {.cmd = COMMANDS,                  \
                                             .cmd_len = array_len(COMMANDS),   \
                                             .manual = MANUAL,                 \
                                             .manual_len = array_len(MANUAL)}

#define make_main(SETTINGS, COMMAND)                                           \
        int main(const int argc, Args argv) {                                  \
                Vec cmd = new_vec();                                           \
                push(&cmd, COMMAND);                                           \
                run_cli((size_t)argc, argv, &SETTINGS, &cmd);                  \
        }

#define make_single(...)                                                       \
        static const Cmd CMD = cmd(__VA_ARGS__);                               \
        int main(const int argc, Args argv) {                                  \
                run_cli_single((size_t)argc, argv, &CMD);                      \
        }

__nonnull() void print_vec(Vec *vec);
