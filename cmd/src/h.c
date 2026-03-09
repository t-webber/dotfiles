#include "libcmd.h"

const Cmd COMMANDS[] = {
    cmd("a",
        "auth",
        "=ilogin",
        "=ologout",
        "=usetup-git",
        "--hostname",
        "refresh",
        "-h",
        "github.com",
        "-s",
        "deleteèrepo", ),
    cmd("p", "pr", "=ncreate", "view", "-w", ),
    cmd("r",
        "repo",
        "=ncreate",
        "=oclone",
        "set-default",
        "delete",
        "view",
        "-w",
        "--branch", ),
    cmd("u", "run", "list", "view", "=L--limit", "--json", ),
};

const Manual MANUAL[] = {{'?', "--help"}};

make_settings(SETTINGS, COMMANDS, MANUAL);
make_main(SETTINGS, "gh")
