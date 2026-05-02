#include "lib.h"
#include "libcmd.h"

__attribute_malloc__ __wur static char *cmd_head_num(const int value) {
        char *ret = malloc(64);
        sprintf(ret, "HEAD~%d", value);
        return ret;
}

#define cmd_num(name, alias)                                                   \
        __attribute_malloc__ __wur static char *cmd_##name##_num(              \
            const int value) {                                                 \
                if (strcmp(get_last_pushed(), #alias))                         \
                        return cmd_head_num(value);                            \
                char *const num = malloc(64);                                  \
                sprintf(num, "%d", value);                                     \
                return num;                                                    \
        }

cmd_num(l, n) cmd_num(cl, d)

#undef cmd
#define cmd(alias, expanded, ...)                                              \
        cmdfunc(alias, expanded, cmd_head_num, __VA_ARGS__)

    /// TODO: add a iorb to rebase edit on one commit only w/o editor
    /// TODO: on git rebase conflicts, display the list of CONFLICTS only (and
    /// put the full log i*constn a file in del?)
    /// TODO: same for git merge
    /// TODO: interactive git add that let's me select files to add.

    static const Cmd COMMANDS[] = {
        cmd("a", "add", "-p", ".", "--intent-to-add", "-e", "--all", ),

        cmd("br",
            "branch",
            "-a",
            "-d",
            "--force",
            "-D",
            "-m",
            "-M",
            "-r",
            "-u",
            "--contains",
            "--show-current", ),
        cmd("bi",
            "bisect",
            "start",
            "bad",
            "good",
            "run",
            "=ereset",
            "visualize",
            "=a--stat", ),

        cmdfunc("cl", "clone", cmd_cl_num, "--depth", ),
        cmd("ck", "checkout", "-t", "--orphan", "-b", ),
        cmd("cp", "cherry-pick", "--abort", "--continue", ),
        cmd("cg",
            "config",
            "--add",
            "credential.helper",
            "diff.tool",
            "=euser.email",
            "false",
            "--global",
            "=hurl.\"https://github.com/\".insteadOf",
            "init.defaultBranch",
            "--list",
            "=nuser.name",
            "=odifftool.prompt",
            "pull.rebase",
            "--replace-all",
            "set",
            "=Scommit.gpgsign",
            "true",
            "=upush.autoSetupRemote",
            "=vnvimdiff",
            "=wurl.\"https://github.com/t-webber/\".insteadOf", ),
        cmd("c",
            "commit",
            "-m",
            "--amend",
            "-q",
            "-S",
            "--no-edit",
            "--quiet",
            "=b--allow-empty",
            "=g--allow-empty-message",
            "--reset-author", ),

        cmd("do", "difftool", "--cached", ),
        cmd("d", "diff", "--shortstat", "--cached", ),

        cmd("e", "fetch", "--all", "--unshallow", "--prune", "--tags", ),

        cmd("f", "fsck", "--no-reflogs", "--lost-found", ),

        cmd("gc", "gc", "--prune=now", "--aggressive", ),
        cmd("g", "grep", "-F", "--all-match", "-n", ),

        cmd("help", "status", ),

        cmd("i", "init", ),

        cmd("ls", "ls-files", ),
        cmdfunc("lr",
                "reflog",
                cmd_l_num,
                "--graph",
                "-n",
                "expire",
                "=w--expire=now",
                "--all",
                "-p",
                "=a--name-status", ),
        cmdfunc("l",
                "log",
                cmd_l_num,
                "--oneline",
                "--graph",
                "-n",
                "-p",
                "=a--name-status",
                "--stat", ),

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
        cmd("rp", "rev-parse", "--show-toplevel", "--git-common-dir", ),
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
            "--keep-index",
            "=w--all",
            "=S--staged", ),
        cmd("sm",
            "submodule",
            "update",
            "--init",
            "--recursive",
            "d",
            "deinit",
            "-f",
            "-all",
            "--cache", ),
        cmd("sw", "switch", "-C", "-c", ),
        cmd("so", "show", "--stat", ),
        cmd("s", "status", "--ignored", ),

        cmd("t", "tag", "-d", ),

        cmd("v", "version", ),

        cmd("w", "worktree", "add", "remove", "move", "list", "-f", ),

        cmd("y",
            "apply",
            "-R",
            "=s--ignore-space-change",
            "=w--ignore-whitespace",
            "--reject", ),
};

static const Manual MANUAL[] = {{'H', "HEAD"},
                                {'F', "FETCH_HEAD"},
                                {'^', "HEAD"},
                                {'?', "--help"},
                                {'O', "origin"},
                                {'U', "upstream"},
                                {'E', "master"},
                                {'C', "--color=always"},
                                {'-', "--"},
                                {'.', ".."},
                                {'M', "main"},
                                {'P', "--no-pager"},
                                {'G', "--paginate"}};

make_settings(SETTINGS, COMMANDS, MANUAL);

make_main(SETTINGS, "git")
