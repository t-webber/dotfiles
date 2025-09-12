#include "lib.h"
#include "libcmd.h"
#include "libexec.h"
#include "libvec.h"
#include <assert.h>
#include <string.h>

const Cmd COMMANDS[] = {
    cmd(a, add, "p", ".", ),

    cmd(b,
        branch,
        "-a",
        "-d",
        "-D",
        "-m",
        "-M",
        "-r",
        "--set-upstream-to",
        "-u", ),

    cmd(ck, checkout, "-t", "--orphan", "FETCH_HEAD", "HEAD^", ),
    cmd(c, commit, "-m", "--amend", "-q", "-S", "--no-edit", "--allow-empty", ),

    cmd(d, diff, "--shortstat", "--staged", "--cached", ),

    cmd(e, fetch, "-a", "--unshallow", ),

    cmd(i, init),

    cmd(lr, reflog),
    // clang-format off
    cmd(ls, ls-files),
    // clang-format on
    cmd(l, log, "--oneline", "--graph", "-n", ),

    cmd(m, merge, "--allow-unrelated-histories", ),

    cmd(n, clean, "-f", "-f", "-d", "-x", ),

    cmd(ph, push, "-f", "-d", "-u", "--tags", ),
    cmd(pl, pull, "-a", "-f", ),

    cmd(rb,
        rebase,
        "--abort",
        "--continue",
        "--edit-todo",
        "--quit",
        "-i",
        "--root", ),
    cmd(ro, restore, "-S", "-W", ),
    cmd(rs, reset, "--hard", ),
    cmd(rt, remote, "-v", "set-url", "add", ),
    cmd(rw, review, "-d", ),

    cmd(sh, stash, "pop", "drop", ),
    cmd(sm, submodule, "update", "--init", "--recursive", ),
    cmd(ss, status, ),
    cmd(sw, switch, "-C", "-c", ),

    cmd(t, tag, "-d", ),

    cmd(w, worktree, "add", "remove", "move", "list", "-f", )

};

const size_t COMMANDS_LEN = sizeof(COMMANDS) / sizeof(COMMANDS[0]);

// Handle conflicts
// If correct case not found, go to lower ? no but force lower if possible in
// the code

// Leading:
// ^ for HEAD
// ! for nopager
// F for FETCH_HEAD
// \d for HEAD~\d
// m
// mm
// om
// omm
// um
// umm
// osm
// osmm
// usm
// usmm
// o
// u (but not if defined ? conflict with -u)
//
static size_t find_command(const_str arg, size_t *start) {
        for (size_t i = 0; i < COMMANDS_LEN; ++i) {
                size_t n = strlen(COMMANDS[i].alias);
                if (!strncmp(arg, COMMANDS[i].alias, n)) {
                        *start = n;
                        return i;
                }
        };
        upanic("Invalid prefix in arg: %s\n", arg);
}

static bool
take_two(Vec *const cmd, Args opts, const char first, const char second) {
        for (int i = 0; opts[i]; ++i) {
                if (strncmp(opts[i], "--", 2) || opts[i][2] != first) continue;
                char *last = strrchr(opts[i] + 2, '-');
                if (!last || !*++last || *last != second) continue;

                push(cmd, opts[i]);
                return true;
        }

        return false;
}

static bool take_one(Vec *const cmd, Args opts, const char c) {
        for (int i = 0; opts[i]; ++i) {
                const char *first = opts[i];
                for (; *first == '-'; ++first);
                if (*first == c) {
                        push(cmd, opts[i]);
                        return true;
                }
        }

        return false;
}

static char next_is(const char next,
                    const char *const arr,
                    size_t *const index,
                    const size_t end) {
        if (++*index < end) {}
}

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
                bool master = (idx + 1 < end && arg[idx + 1] == 'm');
                push(cmd, master ? "master" : "main");
                return master ? 2 : 1;
        }

        return 0;
}

static void parse_alias(Vec *const cmd, const_str arg, const size_t end) {
        size_t start;
        const size_t cmd_id = find_command(arg, &start);
        push(cmd, COMMANDS[cmd_id].expanded);
        Args opts = COMMANDS[cmd_id].options;

        for (size_t i = start; i < end; ++i) {
                if (i + 1 != end && take_two(cmd, opts, arg[i], arg[i + 1])) {
                        ++i;
                        continue;
                }
                if (take_one(cmd, opts, arg[i])) continue;

                size_t count = take_remote_branch(cmd, arg, i, end);
                if (count) {
                        i += count - 1;
                        continue;
                }

                upanic("Invalid letter: %c.", arg[i]);
        }
}

int main(int argc, Args argv) {
        if (argc == 1) {
                print_cmds(COMMANDS, COMMANDS_LEN);
                upanic("Missing arguments\n");
        }

        Vec cmd = new_vec();
        push(&cmd, "git");

        parse_alias(&cmd, argv[1], strlen(argv[1]));

        reserve(&cmd, (size_t)(argc - 2));
        for (int i = 2; i < argc; ++i) { push(&cmd, argv[i]); }

        push(&cmd, NULL);

        exv(cmd.data);
}
