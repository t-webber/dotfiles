#include "lib.h"
#include "libexec.h"
#include "libos.h"
#include "libvec.h"

typedef struct {
        bool dash;
        bool speed;
        bool file;
} state;

static void parse_arg(const_str arg,
                      Vec *const compile,
                      Vec *const run,
                      state *const st,
                      const_str out) {
        if (st->dash) {
                push_v(run, arg);
                return;
        }

        if (st->file) {
                if (!strcmp(arg, "-")) {
                        push_v(run, out);
                        st->dash = true;
                } else {
                        push_v(compile, arg);
                }
                return;
        }

        if (is_file(arg)) {
                push_v(compile, arg);
                st->file = true;
        } else if (st->speed) {
                upanic("Expected file, found %s", arg);
        } else if (!strcmp(arg, "f")) {
                push_v(compile, "-Ofast");
                push_v(compile, "-s");
                push_v(compile, "-march=native");
                push_v(compile, "-mtune=native");
                st->speed = true;
        } else if (!strcmp(arg, "r")) {
                push_v(compile, "-O3");
                st->speed = true;
        } else if (!strcmp(arg, "d")) {
                push_v(compile, "-O0");
                push_v(compile, "-g");
                st->speed = true;
        } else if (!strcmp(arg, "g")) {
                st->speed = true;
                push_v(compile, "-O0");
                push_v(compile, "-g");
                push_v(run, "gdb");
                push_v(run, "-q");
        } else {
                upanic("Invalid option: %s is not one of "
                       "<file>|r|d|f|g",
                       arg);
        }
}

int main(const int argc, Args argv) {
        store_usage(argv[0], "", false);

        char template[] = "/tmp/gd_XXXXXX";
        int fd = mkstemp(template);
        if (fd <= 0) epanic("Failed to mktemp");
        close(fd);

        Vec compile = new_v();
        push_v(&compile, "cc");
        Vec run = new_v();

        state st = {0};

        for (int i = 1; i < argc; ++i)
                parse_arg(argv[i], &compile, &run, &st, template);

        if (!st.dash) push_v(&run, template);
        if (!st.file) upanic("Missng file");

        push_v(&compile, "-o");
        push_v(&compile, template);
        push_v(&compile, NULL);
        push_v(&run, NULL);
        forked_exvd(compile.data);
        exvd(run.data);
}
