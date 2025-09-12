#include "lib.h"
#include "libcmd.h"
#include "libvec.h"

/// Git improvement ideas:
///
/// - add $$ support for in-first-arg params
/// - add a iorb to rebase edit on one commit only w/o editor
/// - on git rebase conflicts, display the list of CONFLICTS only (and put the
/// full log in a file in del?)
/// - same for git merge
/// - interactive git add that let's me select files to add.

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
    cmd("ck", "checkout", "-t", "--orphan", "-b", ),
    cmd("cp", "cherry-pick", "--abort", "--continue", ),
    cmd("cg",
        "config",
        "--add",
        "credential.helper",
        "=euser.email",
        "false",
        "--global",
        "init.defaultBranch",
        "--list",
        "=nuser.name",
        "pull.rebase",
        "--replace-all",
        "set",
        "=Scommit.gpgsign",
        "true", ),
    cmd("c",
        "commit",
        "-m",
        "--amend",
        "-q",
        "-S",
        "--no-edit",
        "--quiet",
        "=y--allow-empty",
        "=w--allow-empty-message",
        "--reset-author", ),

    cmd("d", "diff", "--shortstat", "--cached", "=.-- .", ),

    cmd("e", "fetch", "--all", "--unshallow", "--prune", "--tags", ),
    cmd("f", "fsck", "--no-reflogs", "--lost-found", ),
    cmd("g", "grep", "-F", "--all-match", "-n", ),
    cmd("gc", "gc", "--prime=now", "--aggressive", ),

    cmd("i", "init", ),

    cmd("ls", "ls-files", ),
    cmd("lr", "reflog", "--graph", "-n", "expire", "=w--expire=now", "--all", ),
    cmd("l", "log", "--oneline", "--graph", "-n", ),

    cmd("mv", "move", ),
    cmd("m",
        "merge",
        "=w--allow-unrelated-histories",
        "--abort",
        "--continue", ),

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
    cmd("rm", "rm", "-f", "-r", "--cached", ),
    cmd("ro", "restore", "-S", "-W", "--staged", ".", ),
    cmd("rs", "reset", "--hard", "--soft", ".", ),
    cmd("rt", "remote", "-v", "set-url", "add", "=wshow", "remove", ),
    cmd("rp", "rev-parse", "--show-toplevel", ),
    cmd("rl", "rev-list", "--all", ),
    cmd("rw", "review", "-d", ),

    cmd("sh",
        "stash",
        "pop",
        "drop",
        "clear",
        "show",
        "apply",
        "list",
        "-m",
        "=S--staged", ),
    cmd("sm",
        "submodule",
        "update",
        "--init",
        "--recursive",
        "add",
        "deinit",
        "-f", ),
    cmd("sw", "switch", "-C", "-c", ),
    cmd("so", "show", "--stat", ),
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
