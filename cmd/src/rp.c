#include "libcmd.h"

const Cmd COMMANDS[] = {
    cmd("t",
        "toolchain",

        "list",
        "--quiet",

        "uninstall",
        "stable",
        "nightly",

        "install",
        "--components",
        "--target",
        "=o--no-self-update",
        "--force",
        "--allow-downgrade", ),
    cmd("c",
        "component",

        "list",
        "--installed",
        "--quiet",
        "--toolchain",

        "add",
        "=g--target",

        "remove", ),
    cmd("g",
        "target",
        "list",
        "add",
        "remove",
        "--toolchain",
        "--installed",
        "--quiet", ),
    cmd("d", "default", "stable", "nightly", ),
    cmd("u", "update", "--no-self-update", "--force", ),
    cmd("v", "version", ),
    cmd("o", "doc", "std", ),
};

const Manual MANUAL[] = {
    {'?', "--help"},
    {'C', "--color=always"},
    {'-', "--"},
};

make_settings(SETTINGS, COMMANDS, MANUAL);
make_main(SETTINGS, "rustup")
