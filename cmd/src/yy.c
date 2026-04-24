#include "lib.h"
#include "libexec.h"
#include "libos.h"
#include "libvec.h"
#include <stdarg.h>

// TODO: expand file to most probable outcome

#include <stdio.h>
#include <stdlib.h>
static const_str USAGE;

#define usage(...)                                                             \
        {                                                                      \
                char err[512];                                                 \
                char *end = stpcpy(err, USAGE);                                \
                *end++ = '\n';                                                 \
                sprintf(end, __VA_ARGS__);                                     \
                upanic("%s", err);                                             \
        }
_Noreturn static void extend_exec(Vec *cmd, Args args, const size_t len) {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
        extend_v(cmd, (const char **)args, len);
#pragma GCC diagnostic pop

        for (size_t i = 0; i < len; ++i) {
                if (args[i] != NULL && !strcmp(args[i], "@")) {
                        print_inline_array(cmd->data);
                        exit(0);
                }
        }

        if (cmd->len <= 0) upanic("Invalid length");

        if (last_v(cmd) != NULL) upanic("Missing leading NULL");

        exvd(cmd->data);
}

_Noreturn static void manual_extend_exec(Args args, const size_t len, ...) {
        va_list manual;
        va_start(manual, len);
        Vec cmd = new_v();
        while (1) {
                const_str next = va_arg(manual, const char *);
                if (next == NULL) break;
                push_v(&cmd, next);
        }

        extend_exec(&cmd, args, len);
}

__wur static char *find_file(const char *const name) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
        if (is_file(name)) return (char *)name;
#pragma GCC diagnostic pop
        read_simple_exldn(64, buf, "ffile", name);
        if (buf == NULL || buf[0] == '\0') usage("File %s not found", name);
        return buf;
}

__nonnull() static __wur bool try_parse_py_opts(Vec *cmd, const_str opts) {
        char state = 0;
        for (const char *ch = opts; *ch != '\0'; ++ch) {
                switch (*ch) {
                case 'u':
                        state |= 1;
                        break;
                case 'i':
                        state |= 2;
                        break;
                case '!':
                        state |= 4;
                        break;
                default:
                        return false;
                }
        }
        if (state & 1) push_v(cmd, "-u");
        if (state & 2) push_v(cmd, "-i");
        if (state & 4) {
                printf("\033c");
                fflush(stdout);
        }
        return true;
}

__wur static char *get_venv(const_str prog) {
        char *const path = malloc(64);
        char *s = path;
        if (is_dir(".venv"))
                s = stpcpy(s, ".venv/bin/");
        else if (is_dir("venv"))
                s = stpcpy(s, "venv/bin/");
        stpcpy(s, prog);
        return s;
}

_Noreturn static void python_file(const int argc, Args argv) {
        Vec cmd = new_v();
        push_v(&cmd, get_venv("python"));
        const int file_idx = try_parse_py_opts(&cmd, argv[1]) ? 2 : 1;
        if (argc <= file_idx) usage("Missing arguments for y [u,i] <file.py>");

        push_v(&cmd, find_file(argv[file_idx]));

        extend_exec(&cmd, argv + file_idx + 1, (size_t)(argc - file_idx));
}

_Noreturn __nonnull() static void serve(Args args) {
        Vec cmd = new_v();
        push_v(&cmd, get_venv("python"));
        push_v(&cmd, "-m");
        push_v(&cmd, "http.server");
        for (const char *const *arg = args; *arg != NULL; ++arg) {
                bool isnum = true;
                for (const char *ch = *arg; *ch; ++ch) {
                        if (*ch < '0' || *ch > '9') {
                                isnum = false;
                                break;
                        };
                }
                if (isnum) {
                        push_v(&cmd, *arg);
                } else {
                        push_v(&cmd, "-b");
                        push_v(&cmd, *arg);
                }
        }
        push_v(&cmd, NULL);

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
        NL "\tInteractive Unbuffered"
        NL PROG("y") S CYAN "k" S OPT(PLACE("module"))
        NL PROG("y") S OPT("n,y,i,r,a,v,t,c,e,p,s,m") OPTS
        NL "\tveNv sYnc Init Rm Add serVe pyTest C rEpl Pip Streamlit Module"
        NL PROG("y") S PLACE("module") OPTS;
// clang-format on

int main(const int argc, Args argv) {

        store_usage(argv[0], "", false);

#define exm(offset, prog, ...)                                                 \
        manual_extend_exec(argv + offset,                                      \
                           (size_t)argc - offset + 1,                          \
                           get_venv(prog),                                     \
                           __VA_ARGS__,                                        \
                           NULL)
#define uv(...) exm(2, "uv", __VA_ARGS__)
#define ex1(offset, prog)                                                      \
        manual_extend_exec(argv + offset,                                      \
                           (size_t)argc - offset + 1,                          \
                           get_venv(prog),                                     \
                           NULL);

        if (argc == 1 || (argc == 2 && !strcmp(argv[1], "@")))
                upanic("%s", USAGE);

        if (argc > 1 && argv[1][1] != '\0') python_file(argc, argv);

        if (argc == 2 && !strcmp(argv[1], "c")) new_del();
        if (argc > 2 && !strcmp(argv[1], "k")) uv("lock", "--upgrade-package");
        if (argc == 2 && !strcmp(argv[1], "k")) uv("lock", "--upgrade");

        if (argc > 1 && !strcmp(argv[1], "s")) exm(2, "streamlit", "run");
        if (argc > 1 && !strcmp(argv[1], "n")) uv("venv");
        if (argc > 1 && !strcmp(argv[1], "i")) uv("init");
        if (argc > 1 && !strcmp(argv[1], "r")) uv("remove");
        if (argc > 1 && !strcmp(argv[1], "y")) uv("sync");
        if (argc > 1 && !strcmp(argv[1], "v")) serve(argv + 2);
        if (argc > 1 && !strcmp(argv[1], "e")) ex1(2, "python");
        if (argc > 1 && !strcmp(argv[1], "c")) exm(2, "python", "-c");
        if (argc > 1 && !strcmp(argv[1], "t")) uv("run", "pytest");
        if (argc > 1 && !strcmp(argv[1], "a")) uv("add");
        if (argc > 1 && !strcmp(argv[1], "m")) exm(2, "python", "-m");

        uv("add");
}
