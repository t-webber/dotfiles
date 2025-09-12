#include "libcmd.h"

const Cmd COMMANDS[] = {
    cmd("a", "auth", "=ilogin", "=ologout", "setup-git", "--hostname", ),
    cmd("p", "pr", "=ncreate", "view", "-w", ),
    cmd("r",
        "repo",
        "=ncreate",
        "=oclone",
        "=dset-default",
        "view",
        "-w",
        "--branch", ),
};

const Manual MANUAL[] = {{'?', "--help"}};

make_settings(SETTINGS, COMMANDS, MANUAL);
make_main(SETTINGS, "gh")
