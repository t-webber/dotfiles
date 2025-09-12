#pragma once

#include "lib.h"
#include "libvec.h"
#include <stddef.h>

#define cmd(alias, expanded, ...)                                              \
        {                                                                      \
                alias, expanded, (const char *[]) { __VA_ARGS__ NULL }         \
        }

typedef struct {
        const char *alias;
        const char *expanded;
        const char **options;
} Cmd;

void print_cmds(const Cmd *const cmd, const size_t cmd_len);

size_t find_command(const_str arg,
                    size_t *start,
                    const Cmd *const cmds,
                    const size_t len);

bool take_two(Vec *const cmd, Args opts, const char first, const char second);
bool take_one(Vec *const cmd, Args opts, const char c);
