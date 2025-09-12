#include "lib.h"
#include "libexec.h"
#include "libos.h"
#include "libvec.h"

static const_str USAGE;

#define usage(...)                                                             \
        {                                                                      \
                char err[512];                                                 \
                char *end = stpcpy(err, USAGE);                                \
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

__nonnull((1, 2)) _Noreturn static void python(const_str file,
                                               Args argv,
                                               const_str opts) {
        Vec cmd = new_vec();
        push(&cmd, get_python());

        if (opts) { parse_python_opts(&cmd, opts); }

        push(&cmd, file);
        for (const char *const *arg = argv; *arg != NULL; ++arg)
                push(&cmd, *arg);
        push(&cmd, NULL);

        exvd(cmd.data);
}

_Noreturn static void uv(const_str command, Args args) {
        Vec cmd = new_vec();
        push(&cmd, "uv");
        push(&cmd, command);
        for (const char *const *arg = args; *arg != NULL; ++arg)
                push(&cmd, *arg);

        exvd(cmd.data);
}

_Noreturn static void serve(Args args) {
        Vec cmd = new_vec();
        push(&cmd, get_python());
        push(&cmd, "-m");
        push(&cmd, "http.server");
        for (const char *const *arg = args; *arg != NULL; ++arg) {
                bool isnum = true;
                for (const char *ch = *arg; *ch; ++ch) {
                        if (*ch > '0' && *ch < '9') {
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

static const_str USAGE = RED
    "Usage:\n" MAGENTA " - y [u,i] " YELLOW "<file.py>" MAGENTA " [OPTIONS]\n"
    " - y [n,y,i,p,a,v] [OPTIONS]\n"
    " - y " YELLOW "<python-module>" MAGENTA " [OPTIONS]\n" RED;

int main(const int argc, Args argv) {

        store_usage(argv[0], "", false);

        if (argc == 1) usage("Missing arguments");

        if (argc > 1 && is_file(argv[1])) python(argv[1], argv + 2, NULL);
        if (argc > 2 && is_file(argv[2])) python(argv[2], argv + 3, argv[1]);

        if (argc > 1 && !strcmp(argv[1], "n")) uv("venv", argv + 2);
        if (argc > 1 && !strcmp(argv[1], "i")) uv("init", argv + 2);
        if (argc > 1 && !strcmp(argv[1], "y")) uv("sync", argv + 2);
        if (argc > 1 && !strcmp(argv[1], "p")) uv("python", argv + 2);
        if (argc > 1 && !strcmp(argv[1], "v")) serve(argv + 2);

        if (argc > 2 && !strcmp(argv[1], "a")) uv("add", argv + 2);

        uv("add", argv + 2);
}
