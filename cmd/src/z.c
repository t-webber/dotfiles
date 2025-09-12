#include "libcmd.h"

const Cmd COMMANDS[] = {
    cmd("b", "build", "run", "test", ),
    cmd("i", "init", ),
};

const Manual MANUAL[] = {
    {'?', "--help"},
    {'C', "--color=always"},
    {'-', "--"},
};

make_settings(SETTINGS, COMMANDS, MANUAL);
make_main(SETTINGS, "zig")
