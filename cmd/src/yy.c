#include "lib.h"
#include "libexec.h"
#include "libos.h"
#include "libvec.h"

#include <stdio.h>
static const_str USAGE;

#define usage(...)                                                             \
        {                                                                      \
                char err[512];                                                 \
                char *end = stpcpy(err, USAGE);                                \
                *end++ = '\n';                                                 \
                sprintf(end, __VA_ARGS__);                                     \
                upanic("%s", err);                                             \
        }

__nonnull() static void parse_python_opts(Vec *cmd, const_str opts) {
        if (!strcmp(opts, "_")) return;
        for (const char *ch = opts; *ch != '\0'; ++ch) {
                switch (*ch) {
                case 'u':
                        push(cmd, "-u");
                        break;
                case 'i':
                        push(cmd, "-i");
                        break;
                default:
                        usage("Invalid option char: " GREEN "%c" RED ".", *ch);
                }
        }
}

__wur static const char *get_python(void) {
        if (is_dir(".venv"))
                return ".venv/bin/python";
        else if (is_dir("venv"))
                return "venv/bin/python";
        else
                return "python3";
}

__nonnull((2)) _Noreturn static void python(const_str file,
                                            Args argv,
                                            const_str opts) {
        Vec cmd = new_vec();
        push(&cmd, get_python());

        if (opts) { parse_python_opts(&cmd, opts); }

        if (file) push(&cmd, file);
        for (const char *const *arg = argv; *arg != NULL; ++arg)
                push(&cmd, *arg);
        push(&cmd, NULL);

        exvd(cmd.data);
}

_Noreturn __nonnull((1, 3)) static void uv(const_str command,
                                           const_str opt,
                                           Args args) {
        Vec cmd = new_vec();
        push(&cmd, "uv");
        push(&cmd, command);
        if (opt != NULL) { push(&cmd, opt); }
        for (const char *const *arg = args; *arg != NULL; ++arg)
                push(&cmd, *arg);

        exvd(cmd.data);
}

_Noreturn __nonnull() static void serve(Args args) {
        Vec cmd = new_vec();
        push(&cmd, get_python());
        push(&cmd, "-m");
        push(&cmd, "http.server");
        for (const char *const *arg = args; *arg != NULL; ++arg) {
                bool isnum = true;
                for (const char *ch = *arg; *ch; ++ch) {
                        if (*ch < '0' || *ch > '9') {
                                isnum = false;
                                break;
                        };
                }
                if (isnum) {
                        push(&cmd, *arg);
                } else {
                        push(&cmd, "-b");
                        push(&cmd, *arg);
                }
        }
        push(&cmd, NULL);

        exvd(cmd.data);
}

_Noreturn static void new_del(void) {
        const bool in_venv = is_dir(".venv");
#define PATH(x) in_venv ? ".venv/bin/" x : x
        const_str path = PATH(".del.py");
        const_str python = PATH("python3");
#undef PATH
        if (!fork_and_wait()) exldn("nvim", path);
        exldn(python, path);
}

#define NL "\n" CYAN " - "

// clang-format off
static const_str USAGE = U
        NL PROG("y")
        NL PROG("y") S OPT("u,i") S PLACE("file.py") OPTS 
        NL PROG("y") S CYAN "k" S OPT(PLACE("module"))
        NL PROG("y") S OPT("n,y,i,m,r,a,v,t,c") OPTS
        NL "\tveNv sYnc Init python-Mgr python-Run Add serVe pyTest C"
        NL PROG("y") S PLACE("module") OPTS;
// clang-format on

int main(const int argc, Args argv) {

        store_usage(argv[0], "", false);

        if (argc == 1) usage("Missing arguments");
        for (int i = 1; i < argc; ++i)
                if (!strcmp(argv[i], "@")) upanic("%s", USAGE);

        if (argc > 1 && is_file(argv[1])) python(argv[1], argv + 2, NULL);
        if (argc > 2 && is_file(argv[2])) python(argv[2], argv + 3, argv[1]);

        if (argc > 1 && !strcmp(argv[1], "n")) uv("venv", NULL, argv + 2);
        if (argc > 1 && !strcmp(argv[1], "i")) uv("init", NULL, argv + 2);
        if (argc > 1 && !strcmp(argv[1], "y")) uv("sync", NULL, argv + 2);
        if (argc > 1 && !strcmp(argv[1], "m")) uv("python", NULL, argv + 2);
        if (argc > 1 && !strcmp(argv[1], "k"))
                uv("lock", "--upgrade", argv + 2);
        if (argc > 1 && !strcmp(argv[1], "kp"))
                uv("lock", "--upgrade-package", argv + 2);
        if (argc > 1 && !strcmp(argv[1], "v")) serve(argv + 2);
        if (argc > 1 && !strcmp(argv[1], "r")) python(NULL, argv + 2, NULL);
        if (argc == 2 && !strcmp(argv[1], "c")) new_del();
        if (argc > 1 && !strcmp(argv[1], "c")) python("-c", argv + 2, NULL);
        if (argc > 1 && !strcmp(argv[1], "t")) uv("run", "pytest", argv + 2);

        if (argc > 2 && !strcmp(argv[1], "a")) uv("add", NULL, argv + 2);

        uv("add", NULL, argv + 2);
}
