#include "lib.h"
#include "libcmd.h"
#include "libexec.h"
#include "libvec.h"
#include <assert.h>
#include <ctype.h>
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
        "--set-upstream-to",
        "-u", ),
    cmd("bi", "bisect", "start", "bad", "good", "run", "=ereset", ),

    cmd("ck", "checkout", "-t", "=p--orphan", ),
    cmd("cp", "cherry-pick", "--abort", ),
    cmd("cg", "config", "--global", ),
    cmd("c",
        "commit",
        "-m",
        "--amend",
        "-q",
        "-S",
        "--no-edit",
        "--quiet",
        "=y--allow-empty", ),

    cmd("d", "diff", "--shortstat", "--cached", "=.-- .", ),

    cmd("e", "fetch", "--all", "--unshallow", ),

    cmd("i", "init", ),

    cmd("lr", "reflog", ),
    // clang-format off
    cmd("ls", "ls-files", ),
    // clang-format on
    cmd("l", "log", "--oneline", "--graph", "-n", ),

    cmd("m", "merge", "--allow-unrelated-histories", ),

    cmd("n", "clean", "-f", "-f", "-d", "-x", ),

    cmd("ph",
        "push",
        "=F--force",
        "=f--force-with-lease",
        "-d",
        "-u",
        "--quiet",
        "--tags", ),
    cmd("pl", "pull", "-a", "-f", "--quiet", ),

    cmd("rb",
        "rebase",
        "--abort",
        "--continue",
        "--edit-todo",
        "--quit",
        "-i",
        "--root", ),
    cmd("rm", "remove", "-f", ),
    cmd("ro", "restore", "-S", "-W", ),
    cmd("rs", "reset", "--hard", ),
    cmd("rt", "remote", "-v", "set-url", "add", ),
    cmd("rp", "rev-parse", "--show-toplevel", ),
    cmd("rl", "rev-list", ),
    cmd("rw", "review", "-d", ),

    cmd("sh", "stash", "pop", "drop", "clear", "show", "apply", ),
    cmd("sm", "submodule", "update", "--init", "--recursive", ),
    cmd("ss", "status", ),
    cmd("sw", "switch", "-C", "-c", ),
    cmd("so", "show", ),

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

// Handle conflicts
// If correct case not found, go to lower ? no but force lower if possible in
// the code, conflict upstream/-u

static size_t take_remote_branch(Vec *const cmd,
                                 const_str arg,
                                 size_t idx,
                                 const size_t end) {
        const char first = arg[idx];
        if (first == 'o' || first == 'u') {
                const_str remote = first == 'o' ? "origin" : "upstream";

                if (idx + 1 < end && arg[idx + 1] == 's') {
                        ++idx;
                        assert(++idx < end && arg[idx] == 'm');
                        bool master = (idx + 1 < end && arg[idx + 1] == 'm');
                        char *buf = malloc(sizeof(char) * 32);
                        sprintf(buf,
                                "%s/%s",
                                remote,
                                master ? "master" : "main");
                        push(cmd, buf);
                        return master ? 4 : 3;
                }

                push(cmd, remote);

                if (idx + 1 < end && arg[idx + 1] == 'm') {
                        ++idx;
                        bool master = (idx + 1 < end && arg[idx + 1] == 'm');
                        push(cmd, master ? "master" : "main");
                        return master ? 3 : 2;
                }

                return 1;
        }

        if (first == 'm') {
                ++idx;
                bool master = (idx < end && arg[idx] == 'm');
                push(cmd, master ? "master" : "main");
                return master ? 2 : 1;
        }

        return 0;
}

static void handle_char(Vec *const cmd,
                        const_str arg,
                        Args opts,
                        size_t *i,
                        const size_t end) {

        const char ch = arg[*i];
        size_t count;

        if (isdigit(ch)) {
                size_t len = 0;
                for (; isdigit(*(arg + *i + len)); ++len);
                char *const head_arg = malloc((len + 1 + 5) * sizeof(char));
                sprintf(head_arg, "HEAD~%d", atoi(arg + *i));
                push(cmd, head_arg);
                *i += len - 1;
                return;
        }

        switch (ch) {

        case '!':
                system("echo -en \033c");
                return;

        case '=':
                if (setenv("GIT_PAGER", "", 1) == 0) return;
                epanic("setenv GIT_PAGER failed");

        case 'H':
                push(cmd, "HEAD");
                return;

        case '?':
                push(cmd, "--help");
                return;

        case 'F':
                push(cmd, "FETCH_HEAD");
                return;

        case '^':
                push(cmd, "HEAD^");
                return;

        default:
                break;
        }

        if (*i + 1 != end && take_two(cmd, opts, ch, arg[*i + 1])) {
                ++*i;
        } else if (take_one(cmd, opts, ch)) {
        } else if ((count = take_remote_branch(cmd, arg, *i, end)) != 0) {
                *i += count - 1;
        } else {
                upanic("Invalid letter: %c.\n", ch);
        }
}

static void parse_alias(Vec *const cmd, const_str arg, const size_t end) {
        size_t start;
        const size_t cmd_id = find_command(arg, &start, COMMANDS, COMMANDS_LEN);
        push(cmd, COMMANDS[cmd_id].expanded);
        Args opts = COMMANDS[cmd_id].options;

        for (size_t i = start; i < end; ++i)
                handle_char(cmd, arg, opts, &i, end);
}

int main(int argc, Args argv) {
        if (argc == 1 || (argc == 2 && !strcmp(argv[1], "!"))) {
                if (argc == 2) clear();
                print_cmds(COMMANDS, COMMANDS_LEN);
                printf("\n@: print command\n?: no pager\n!: clear and run\n");
                printf("\nH: HEAD    F: FETCH_HEAD    ^: HEAD^    \\d+: "
                       "HEAD~\\1\n");
                printf("\n{o,u,}{s,}{m,mm}: "
                       "{origin,upstream,}{/, }{main,master}\n");
                return 1;
        }

        Vec cmd = new_vec();
        push(&cmd, "git");

        parse_alias(&cmd, argv[1], strlen(argv[1]));

        bool test = false;

        reserve(&cmd, (size_t)(argc - 2));
        for (int i = 2; i < argc; ++i) {
                if (!strcmp(argv[i], "@")) {
                        test = true;
                } else
                        push(&cmd, argv[i]);
        }

        push(&cmd, NULL);

        if (!test) exv(cmd.data);

        for (size_t i = 0; i + 1 < cmd.len; ++i) { printf("%s ", cmd.data[i]); }

        printf("\n");
}
