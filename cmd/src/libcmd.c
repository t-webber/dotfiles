#include "libcmd.h"
#include "lib.h"
#include "libvec.h"
#include <stdio.h>
#include <string.h>

static void print_cmd(const Cmd *const cmd, const size_t max_alias, const size_t max_expanded) {
        printf("%-*s   %-*s ", max_alias, cmd->alias, max_expanded, cmd->expanded);
        for (const char **rdr = cmd->options; *rdr != NULL; ++rdr) {
                printf("[%s] ", *rdr);
        }
        printf("\n");
}

static size_t get_max_alias(const Cmd *const cmds, const size_t cmd_len) {
        size_t max = 0;
        size_t current;
        for (size_t i = 0; i < cmd_len; ++i) { current = strlen(cmds[i].alias); if (current > max) max = current; }
        return max;
}

static size_t get_max_expanded(const Cmd *const cmds, const size_t cmd_len) {
        size_t max = 0;
        size_t current;
        for (size_t i = 0; i < cmd_len; ++i) { current = strlen(cmds[i].expanded); if (current > max) max = current; }
        return max;
}

void print_cmds(const Cmd *const cmds, const size_t cmd_len) {
        const size_t max_alias = get_max_alias(cmds, cmd_len);
        const size_t max_expanded = get_max_expanded(cmds, cmd_len);
        for (size_t i = 0; i < cmd_len; ++i) { print_cmd(&cmds[i], max_alias, max_expanded); }
}

size_t find_command(const_str arg,
                    size_t *start,
                    const Cmd *const cmds,
                    const size_t len) {
        for (size_t i = 0; i < len; ++i) {
                size_t n = strlen(cmds[i].alias);
                if (!strncmp(arg, cmds[i].alias, n)) {
                        *start = n;
                        return i;
                }
        };
        upanic("Invalid prefix in arg: %s\n", arg);
}

bool take_two(Vec *const cmd, Args opts, const char first, const char second) {
        for (int i = 0; opts[i]; ++i) {
                if (strncmp(opts[i], "--", 2) || opts[i][2] != first) continue;
                char *last = strrchr(opts[i] + 2, '-');
                if (!last || !*++last || *last != second) continue;

                push(cmd, opts[i]);
                return true;
        }

        return false;
}

bool take_one(Vec *const cmd, Args opts, const char c) {
        for (int i = 0; opts[i]; ++i) {
                const char *first = opts[i];
                for (; *first == '-'; ++first)
                        ;
                if (*first == c) {
                        push(cmd, opts[i]);
                        return true;
                }
                if (*first == '=' && *++first == c) {
                        push(cmd, opts[i] + 2);
                        return true;
                }
        }

        return false;
}
