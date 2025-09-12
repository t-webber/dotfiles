#pragma once

#include <stddef.h>

#define cmd(alias, expanded, ...)                                              \
        {                                                                      \
                #alias, #expanded, (const char *[]) {                          \
                        __VA_ARGS__ NULL                                       \
                }                                                              \
        }

typedef struct {
        const char *alias;
        const char *expanded;
        const char **options;
} Cmd;

void print_cmd(const Cmd *const cmd);

void print_cmds(const Cmd *const cmd, const size_t cmd_len);
