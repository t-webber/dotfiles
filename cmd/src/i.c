#include "lib.h"
#include "libcmd.h"
#include "libvec.h"
#include <assert.h>
#include <string.h>

const Cmd COMMANDS[] = {
    cmd("a", "add", "-p", ".", "--intent-to-add", "-e", ),

    cmd("br",
        "branch",
        "-a",
        "-d",
        "-D",
        "-m",
        "-M",
        "-r",
        "-u",
        "--contains", ),
    cmd("bi",
        "bisect",
        "start",
        "bad",
        "good",
        "run",
        "=ereset",
        "visualize",
        "=a--stat", ),

    cmd("cl", "clone", "--depth", ),
    cmd("ck", "checkout", "-t", "--orphan", ),
    cmd("cp", "cherry-pick", "--abort", "--continue", ),
    cmd("cg",
        "config",
        "--global",
        "=euser.email",
        "=nuser.name",
        "=iinit.defaultBranch",
        "=ccredential.helper",
        "--add",
        "--replace-all",
        "--list", ),
    cmd("c",
        "commit",
        "-m",
        "--amend",
        "-q",
        "-S",
        "--no-edit",
        "--quiet",
        "=y--allow-empty",
        "--reset-author", ),

    cmd("d", "diff", "--shortstat", "--cached", "=.-- .", ),

    cmd("e", "fetch", "--all", "--unshallow", "--prune", "--tags", ),
    cmd("f", "fsck", "--no-reflogs", "--lost-found", ),
    cmd("g", "grep", "-F", "--all-match", "-n", ),
    cmd("gc", "gc", "--prime=now", "--aggressive", ),

    cmd("i", "init", ),

    cmd("ls", "ls-files", ),
    cmd("lr", "reflog", "--graph", "-n", "expire", "=w--expire=now", "--all", ),
    cmd("l", "log", "--oneline", "--graph", "-n", "=c--color=always", ),

    cmd("mv", "move", ),
    cmd("m", "merge", "--allow-unrelated-histories", ),

    cmd("n", "clean", "-f", "-f", "-d", "-x", "--recurse-submodules", ),

    cmd("ph",
        "push",
        "=F--force",
        "=f--force-with-lease",
        "-d",
        "-u",
        "--quiet",
        "--tags",
        "=gHEAD:refs/for/master",
        "=GHEAD:refs/for/main", ),
    cmd("pl", "pull", "-a", "-f", "--quiet", ),

    cmd("rb",
        "rebase",
        "--abort",
        "--continue",
        "--edit-todo",
        "--quit",
        "-i",
        "--root", ),
    cmd("rm", "rm", "-f", "-r", ),
    cmd("ro", "restore", "-S", "-W", ),
    cmd("rs", "reset", "--hard", "--soft", ),
    cmd("rt", "remote", "-v", "set-url", "add", "=wshow", ),
    cmd("rp", "rev-parse", "--show-toplevel", ),
    cmd("rl", "rev-list", "--all", ),
    cmd("rw", "review", "-d", ),

    cmd("sh", "stash", "pop", "drop", "clear", "show", "apply", "list", ),
    cmd("sm",
        "submodule",
        "update",
        "--init",
        "--recursive",
        "add",
        "deinit",
        "-f", ),
    cmd("sw", "switch", "-C", "-c", ),
    cmd("so", "show", ),
    cmd("s", "status", ),

    cmd("t", "tag", "-d", ),

    cmd("w", "worktree", "add", "remove", "move", "list", "-f", ),

    cmd("y",
        "apply",
        "-R",
        "=s--ignore-space-change",
        "=w--ignore-whitespace",
        "--reject", ),
};

const size_t COMMANDS_LEN = sizeof(COMMANDS) / sizeof(COMMANDS[0]);

const Manual MANUAL[] = {{'H', "HEAD"},
                         {'F', "FETCH_HEAD"},
                         {'^', "HEAD"},
                         {'?', "--help"},
                         {'O', "origin"},
                         {'U', "upstream"},
                         {'E', "master"},
                         {'C', "--color=always"},
                         {'M', "main"}};

const size_t MANUAL_LEN = sizeof(MANUAL) / sizeof(MANUAL[0]);

const CliSettings SETTINGS = {
    .cmd = COMMANDS,
    .cmd_len = COMMANDS_LEN,
    .manual = MANUAL,
    .manual_len = MANUAL_LEN,
};

int main(const int argc, Args argv) {
        Vec cmd = new_vec();
        push(&cmd, "git");
        run_cli((size_t)argc, argv, &SETTINGS, &cmd);
}
