#include "lib.h"

#define ca(ch, str)                                                            \
        case ch:                                                               \
                fprintf(out, str);                                             \
                return (ch >= '0' && ch <= '9') ? iscolour : otherescape;

#define co(ch, num) ca(ch, "\033[3" #num "m")

enum colour { invalid, iscolour, otherescape };

__nonnull() __wur static enum colour print_escaped(FILE *out, const char ch) {

        switch (ch) {
                co('0', 0);
                co('1', 1);
                co('2', 2);
                co('3', 3);
                co('4', 4);
                co('5', 5);
                co('6', 6);
                co('7', 7);
                co('8', 8);
                co('9', 9);
                ca('t', "\t");
                ca('n', "\n");
                ca('r', "\r");
                ca('%', "%%");
                ca('_', "\033[m");
        default:
                return invalid;
        }
}

__wur __nonnull() static bool print_arg(FILE *out,
                                        const_str arg,
                                        const bool no_escape) {
        bool has_colours = false;
        bool escaped = false;

        for (const char *ptr = arg; *ptr; ++ptr) {
                if (escaped) {
                        enum colour iscol = print_escaped(out, *ptr);
                        if (iscol == invalid)
                                upanic("Invalid escape character '%c' in '%s'",
                                       *ptr,
                                       arg);
                        if (iscol == iscolour) has_colours = true;
                        escaped = false;
                        continue;
                }
                if (*ptr == '%' && !no_escape)
                        escaped = true;
                else
                        fprintf(out, "%c", *ptr);
        }
        return has_colours;
}

int main(const int argc, Args argv) {
        store_usage(argv[0], "", false);

        if (!strcmp(argv[0], "lh")) {
                printf("l: echo <default behaviour>\n"
                       "lh: help\n"
                       "la: wrap <=== $@ ===>\n"
                       "le: error <l %%1$@>\n"
                       "ls: \\n between each arg <ls *>\n"
                       "lw: no  trailing \\n\n"
                       "lr: no escape\n");
                return 1;
        }

        if (!strcmp(argv[0], "lc")) {
                clear();
                return 0;
        }

        bool equals = !strcmp(argv[0], "la");

        if (equals) { printf("\x1b[33m===== "); }

        bool no_new_line = !strcmp(argv[0], "lw");
        bool no_escape = !strcmp(argv[0], "lr");
        bool error = !strcmp(argv[0], "le");
        bool new_line_sep = !strcmp(argv[0], "ls");
        bool has_colours = error;

        FILE *out = error ? stderr : stdout;

        if (error) { fprintf(out, "\x1b[31m"); }

        for (int i = 1; i < argc; ++i) {
                if (i >= 2) fprintf(out, new_line_sep ? "\n" : " ");
                has_colours = print_arg(out, argv[i], no_escape) || has_colours;
        }

        if (equals) { fprintf(out, " \x1b[33m====="); }
        if (has_colours || equals) fprintf(out, "\x1b[0m");

        if (!no_new_line) fprintf(out, "\n");

        return error ? 1 : 0;
}
