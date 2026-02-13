#include "libcmd.h"
#include "lib.h"
#include "libexec.h"
#include "libvec.h"
#include <ctype.h> // IWYU pragma: keep
#include <stdio.h>
#include <string.h>

__nonnull() static void print_manual(const Manual *const manual) {
        printf("%c  %s\n", manual->origin, manual->replace);
}

__nonnull() static void print_cmd(const Cmd *const cmd,
                                  size_t max_alias,
                                  const size_t max_expanded) {
        if (max_alias) ++max_alias;
        printf("%-*s %-*s ",
               (int)max_alias,
               cmd->alias,
               (int)max_expanded,
               cmd->expanded);
        for (const char **rdr = cmd->options; *rdr != NULL; ++rdr) {
                printf("[%s] ", *rdr);
        }
        printf("\n");
}

__wur __nonnull() static size_t
    get_max_alias(const Cmd *const cmds, const size_t cmd_len) {
        size_t max = 0;
        size_t current;
        for (size_t i = 0; i < cmd_len; ++i) {
                current = strlen(cmds[i].alias);
                if (current > max) max = current;
        }
        return max;
}

__wur __nonnull() static size_t
    get_max_expanded(const Cmd *const cmds, const size_t cmd_len) {
        size_t max = 0;
        size_t current;
        for (size_t i = 0; i < cmd_len; ++i) {
                current = strlen(cmds[i].expanded);
                if (current > max) max = current;
        }
        return max;
}

__nonnull() static void print_cmds(const Cmd *const cmds,
                                   const size_t cmd_len) {
        const size_t max_alias = get_max_alias(cmds, cmd_len);
        const size_t max_expanded = get_max_expanded(cmds, cmd_len);
        for (size_t i = 0; i < cmd_len; ++i) {
                print_cmd(&cmds[i], max_alias, max_expanded);
        }
}

__nonnull() static void print_help(const CliSettings *const settings) {
        print_cmds(settings->cmd, settings->cmd_len);
        printf("\
---\n\
!  clear\n\
@  print command\n\
$  toggle raw mode for in-arg-values\n\
=  no pager\n\
/  add / between 2-word arg\n\
:  add : between 2-word arg\n\
,  usage for command\n\
\?  man for command\n\
R  rust backtrace\n\
---\n\
");
        for (size_t i = 0; i < settings->manual_len; ++i) {
                print_manual(&settings->manual[i]);
        }
}

__wur __nonnull() static size_t find_command(const_str arg,
                                             size_t *start,
                                             const Cmd *const cmds,
                                             const size_t len) {
        for (size_t i = 0; i < len; ++i) {
                size_t n = strlen(cmds[i].alias);
                if (!strncmp(arg, cmds[i].alias, n)) {
                        *start = n;
                        return i;
                }
        };
        upanic("Invalid prefix in arg: %s", arg);
}

__nonnull() __wur static bool take_two(Vec *const cmd,
                                       Args opts,
                                       const char first,
                                       const char second) {
        for (int i = 0; opts[i]; ++i) {
                if (strncmp(opts[i], "--", 2) || opts[i][2] != first) continue;
                const char *last = strrchr(opts[i] + 2, '-');
                if (!last || !*++last || *last != second) continue;

                push(cmd, opts[i]);
                return true;
        }

        return false;
}

__nonnull() __wur
    static bool take_one(Vec *const cmd, Args opts, const char c) {
        for (int i = 0; opts[i]; ++i) {
                const char *first = opts[i];
                for (; *first == '-'; ++first);
                if (*first == c) {
                        push(cmd, opts[i]);
                        return true;
                }
                if (*first == '=' && *++first == c) {
                        push(cmd, opts[i] + 2);
                        return true;
                }
        }

        return false;
}

struct CharParsingState {
        char prev_sep;
        char sep;
        bool less;
        char *raw_mode;
};

__nonnull() static void handle_char(Vec *const cmd,
                                    const Manual *const expansions,
                                    const size_t nb_expansions,
                                    const_str arg,
                                    Args opts,
                                    size_t *const i,
                                    const size_t end,
                                    struct CharParsingState *const state) {

        const char ch = arg[*i];

        if (ch == '$') {
                if (state->raw_mode == NULL) {
                        state->raw_mode = NULL; // TODO: NEW_STRING("");
                } else {
                        push(cmd, state->raw_mode);
                        state->raw_mode = NULL;
                }
                return;
        }

        if (state->raw_mode != NULL) {
                // TODO: push char into string
                return;
        }

        if (isdigit(ch)) {
                size_t len = 0;
                for (; isdigit(*(arg + *i + len)); ++len);
                char *const head_arg = malloc((len + 1 + 5) * sizeof(char));
                sprintf(head_arg, "HEAD~%d", atoi(arg + *i));
                push(cmd, head_arg);
                *i += len - 1;
                return;
        }

        if (ch == '!') {
                clear();
                return;
        }

        if (ch == '=') setenv_checked("PAGER", "");

        if (ch == 'R') setenv_checked("RUST_BACKTRACE", "1");

        if (ch == '/' || ch == ':') {
                if (state->sep != '\0') upanic("Found 2 consecutive %c", ch);
                state->sep = ch;
                return;
        }

        if (ch == '%') {
                state->less = true;
                return;
        }

        if (*i + 1 != end && take_two(cmd, opts, ch, arg[*i + 1])) {
                ++*i;
        } else if (take_one(cmd, opts, ch)) {
        } else {
                for (size_t m_idx = 0; m_idx < nb_expansions; ++m_idx) {
                        if (ch == expansions[m_idx].origin) {
                                push(cmd, expansions[m_idx].replace);
                                return;
                        }
                }
                upanic("Invalid letter: %c.", ch);
        }
}

__nonnull() static void parse_alias(const CliSettings *const settings,
                                    Vec *const cmd,
                                    const_str arg,
                                    const size_t end) {

        const bool has_command = settings->cmd_len > 1;
        if (settings->cmd_len == 0)
                upanic("`parse_alias` was given empty settings");

        size_t start = 0;
        size_t cmd_id;

        if (has_command) {
                cmd_id = find_command(arg,
                                      &start,
                                      settings->cmd,
                                      settings->cmd_len);
        } else {
                cmd_id = 0;
        }

        const Cmd current = settings->cmd[cmd_id];

        push(cmd, current.expanded);

        struct CharParsingState state
            = {.sep = '\0', .prev_sep = '\0', .less = false, .raw_mode = NULL};

        for (size_t i = start; i < end; ++i) {
                if (arg[i] == ',') {
                        print_cmd(&current, 0, 0);
                        exit(1);
                }

                handle_char(cmd,
                            settings->manual,
                            settings->manual_len,
                            arg,
                            current.options,
                            &i,
                            end,
                            &state);
                if (state.prev_sep != '\0') {
                        const_str post_slash = pop(cmd);
                        const_str pre_slash = pop(cmd);
                        const size_t len
                            = strlen(pre_slash) + strlen(post_slash) + 1;
                        char *const merged = malloc(sizeof(char) * (len + 1));
                        sprintf(merged,
                                "%s%c%s",
                                pre_slash,
                                state.prev_sep,
                                post_slash);
                        push(cmd, merged);
                        state.prev_sep = '\0';
                }
                if (state.sep != '\0') {
                        state.prev_sep = state.sep;
                        state.sep = '\0';
                };
        }
}

__nonnull() _Noreturn static void print_exit_or_exec(const Vec *const cmd,
                                                     const bool debug) {

        if (!debug) exvd(cmd->data);

        for (size_t i = 0; i + 1 < cmd->len; ++i) {
                printf("%s ", cmd->data[i]);
        }

        printf("\n");
        exit(0);
}

__wur __nonnull() static bool push_others(Vec *const cmd,
                                          const size_t argc,
                                          Args argv) {
        bool debug = false;

        reserve(cmd, (size_t)(argc - 2));
        for (size_t i = 2; i < argc; ++i) {
                if (!strcmp(argv[i], "@")) {
                        debug = true;
                } else
                        push(cmd, argv[i]);
        }

        push(cmd, NULL);

        return debug;
}

__nonnull() _Noreturn void run_cli(const size_t argc,
                                   Args argv,
                                   const CliSettings *const settings,
                                   Vec *const cmd) {
        if (argc == 1 || (argc == 2 && !strcmp(argv[1], "!"))) {
                if (argc == 2) clear();
                print_help(settings);
                exit(1);
        }

        store_usage(argv[0], argv[1], false);

        parse_alias(settings, cmd, argv[1], strlen(argv[1]));

        bool debug = push_others(cmd, argc, argv);

        print_exit_or_exec(cmd, debug);
}

__nonnull() _Noreturn void run_cli_single(const size_t argc,
                                          Args argv,
                                          const Cmd *const command) {
        store_usage(argv[0], argv[1] ? argv[1] : "", false);
        bool should_clear = false, help = false;

        for (size_t idx = 1; idx < argc; ++idx) {
                const_str arg = argv[idx];
                if (!strcmp(arg, "!")) { should_clear = true; }
                if (!strcmp(arg, ",")) { help = true; }
        }

        if (should_clear) clear();
        if (help) {
                print_cmd(command, 0, 0);
                exit(0);
        }

        Vec cmd = new_vec();

#define len 1
        const Manual manual[len] = {{'?', "--help"}};
        const size_t manual_len = len;
#undef len

        const CliSettings settings = {.cmd = command,
                                      .cmd_len = 1,
                                      .manual = manual,
                                      .manual_len = manual_len};

        bool debug = false;

        if (argc >= 2) {
                if (strcmp(argv[1], "_")) {
                        parse_alias(&settings, &cmd, argv[1], strlen(argv[1]));
                } else {
                        push(&cmd, command->expanded);
                }

                debug = push_others(&cmd, argc, argv);

                print_exit_or_exec(&cmd, debug);
        } else {
                exl1(command->expanded);
        }
}

__nonnull() void print_vec(Vec *vec) {
        for (size_t idx = 0; idx < vec->len; ++idx) {
                printf("%s ", vec->data[idx]);
        }
        printf("\n");
}
