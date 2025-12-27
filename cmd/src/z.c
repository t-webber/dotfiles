#include "lib.h"
#include "libcmd.h"
#include "libvec.h"
#include <assert.h>
#include <string.h>

const Cmd COMMANDS[] = {
    cmd("b", "build", "run", "test", ),
    cmd("i", "init", ),
};

const size_t COMMANDS_LEN = sizeof(COMMANDS) / sizeof(COMMANDS[0]);

const Manual MANUAL[] = {
    {'?', "--help"},
    {'C', "--color=always"},
    {'-', "--"},
};

const size_t MANUAL_LEN = sizeof(MANUAL) / sizeof(MANUAL[0]);

static CliSettings SETTINGS = {
    .cmd = COMMANDS,
    .cmd_len = COMMANDS_LEN,
    .manual = MANUAL,
    .manual_len = MANUAL_LEN,
};

int main(const int argc, Args argv) {
        Vec cmd = new_vec();
        push(&cmd, "zig");
        run_cli((size_t)argc, argv, &SETTINGS, &cmd);
}
