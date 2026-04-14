#include "lib.h"
#include "libos.h"

#define ca(ch, str)                                                            \
        case ch:                                                               \
                printf(str);                                                   \
                return (ch >= '0' && ch <= '9') ? iscolour : otherescape;

#define co(ch, num) ca(ch, "\033[3" #num "m")

enum colour { invalid, iscolour, otherescape };

__nonnull() __wur static enum colour print_escaped(const char ch) {
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

__wur __nonnull() static bool print_arg(const_str arg, const bool no_escape) {
        bool has_colours = false;
        bool escaped = false;

        for (const char *ptr = arg; *ptr; ++ptr) {
                if (escaped) {
                        enum colour iscol = print_escaped(*ptr);
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
                        printf("%c", *ptr);
        }
        return has_colours;
}

int main(const int argc, Args argv) {
        store_usage(argv[0], "", false);

        if (!strcmp(argv[0], "lc")) {
                clear();
                return 0;
        }

        bool equals = !strcmp(argv[0], "la");

        if (equals) { printf("\x1b[33m===== "); }

        bool no_new_line = !strcmp(argv[0], "lw");
        bool no_escape = !strcmp(argv[0], "lr");
        bool error = !strcmp(argv[0], "le");
        bool has_colours = error;

        if (error) { printf("\x1b[31m"); }

        for (int i = 1; i < argc; ++i) {
                if (i >= 2) printf(" ");
                has_colours = print_arg(argv[i], no_escape) || has_colours;
        }

        if (equals) { printf(" \x1b[33m====="); }
        if (has_colours || equals) printf("\x1b[0m");

        if (!no_new_line) printf("\n");

        return error ? 1 : 0;
}
