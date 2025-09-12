#include "libcmd.h"
#include <stdio.h>

void print_cmd(const Cmd *const cmd) {
        printf("%s\t%s\t", cmd->alias, cmd->expanded);
        for (const char **rdr = cmd->options; *rdr != NULL; ++rdr) {
                printf("[%s] ", *rdr);
        }
        printf("\n");
}

void print_cmds(const Cmd *const cmd, const size_t cmd_len) {
        for (size_t i = 0; i < cmd_len; ++i) { print_cmd(&cmd[i]); }
}
