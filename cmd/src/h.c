#include "lib.h"
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
    cmd("o", "actions", "list", ),
    cmd("p", "pr", "=ncreate", "view", "-w", ),
    cmd("r",
        "repo",
        "=a--private",
        "--branch",
        "--clone",
        "delete",
        "edit",
        "=ncreate",
        "=oclone",
        "--public",
        "rename",
        "set-default",
        "--template",
        "view",
        "-w",
        "--yes", ),
    cmd("u", "run", "list", "view", "=L--limit", "--json", ),
};

const Manual MANUAL[] = {{'?', "--help"}};

make_settings(SETTINGS, COMMANDS, MANUAL);

int main(const int argc, Args argv) {

        const bool different_git = is_verbose(argv[0], "h", "ht");

        if (different_git)
                setenv_checked("GH_HOST", getenv_checked("GH_PREFERED_HOST"));

        Vec cmd = new_v();
        push_v(&cmd, "gh");
        run_cli((size_t)argc, argv, &SETTINGS, &cmd);
}
