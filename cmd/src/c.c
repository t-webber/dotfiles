#include "lib.h"
#include "libcmd.h"
#include "libvec.h"
#include <assert.h>
#include <string.h>

const Cmd COMMANDS[] = {
    cmd("a", "add", "--features", ),

    cmd("bi", "binstall", "-y", "--git", "--features", ),
    cmd("b", "build", "-r", "--features", "-p", ),

    cmd("ee", "tree", ),
    cmd("e", "expand", ),

    cmd("f", "fmt", "--all", "--unstable-features", "--error-on-unformatted", ),

    cmd("ii", "init", ),
    cmd("iu", "install-update", "-a", "-g", ),
    cmd("i", "install", "--git", "--features", ),

    cmd("lo", "login", ),
    cmd("l", "license", "--direct-deps-only", ),

    cmd("n", "clean", ),

    cmd("o",
        "doc",
        "--all",
        "--open",
        "--document-private-items",
        "--no-deps",
        "-D warnings", ),

    cmd("ph", "publish", ),
    cmd("p", "tarpaulin", "=hHtml", "--doc", "--tests", "--fail-under", "-o", ),

    cmd("rm", "remove", ),
    cmd("r", "run", "-r", "--example", "-p", "--features", ),

    cmd("s", "search", "--limit", ),

    cmd("t",
        "test",
        "--jobs",
        "--features",
        "--release",
        "=n--no-fail-fast",
        "=u--no-run",
        "=c--no-capture", ),

    cmd("ub", "uibless", ),
    cmd("u", "uninstall", ),

    cmd("v", "remove", ),

    cmd("wa", "watch", "=x--exec", "--shell", "--clear", "--delay", ),
    cmd("w", "new", "--lib", ),

    cmd("y",
        "clippy",
        "--all",
        "--fix",
        "=w--allow-dirty",
        "-D warnings",
        "=R-D clippy::restriction", ),
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
        push(&cmd, "cargo");

        if (!strcmp(argv[0], "c")) {
        } else if (!strcmp(argv[0], "cn")) {
                push(&cmd, "+nightly");
        } else if (!strcmp(argv[0], "cs")) {
                push(&cmd, "+stable");
        } else {
                upanic("Invalid arg0: %s\n", argv[0]);
        }

        run_cli((size_t)argc, argv, &SETTINGS, &cmd);
}
