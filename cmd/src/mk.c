#include "lib.h"
#include "libexec.h"
#include "libvec.h"
#include <assert.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define ALIASES_LEN 12

const_str *const ALIASES[ALIASES_LEN] = {
    (const_str[2]){"c", "clean"},

    (const_str[2]){"d", "dev"},
    (const_str[2]){"g", "debug"},
    (const_str[2]){"r", "release"},
    (const_str[2]){"u", "run"},

    (const_str[2]){"ii", "init"},
    (const_str[2]){"i", "install"},

    (const_str[2]){"a", "start"},
    (const_str[2]){"o", "stop"},
    (const_str[2]){"e", "enable"},
    (const_str[2]){"s", "disable"},

    (const_str[2]){"v", "valgrind"},
};

__wur __nonnull() static const char *expand_target(const_str target) {
        const char *expanded_target = target;

        for (size_t i = 0; i < ALIASES_LEN; ++i) {
                const_str *const alias = ALIASES[i];

                if (!strcmp(alias[0], expanded_target)) {
                        expanded_target = alias[1];
                        break;
                }

                if (!strcmp(alias[1], expanded_target))
                        upanic("use m %s", alias[0]);
        }

        return expanded_target;
}

__nonnull() _Noreturn static void make(const_str expanded_target,
                                       const_str makefile,
                                       Args flags,
                                       const size_t len) {

        Vec cmd = new_vec();
        push(&cmd, "make");
        push(&cmd, expanded_target);
        push(&cmd, "-f");
        push(&cmd, makefile);

        extend(&cmd, flags, len);

        exvd(cmd.data);
}

__wur static const char *get_makefile(const_str makefile) {
        if (makefile) { return makefile; }

        if (access("Makefile", F_OK)) { return "Makefile"; }

        DIR *dir = opendir_checked(".");
        struct dirent *entry;
        while ((entry = readdir(dir))) {
                if (!strncmp(entry->d_name, "Makefile", sizeof("Makefile"))) {
                        const_str filename = strdup(entry->d_name);
                        closedir_checked(dir);
                        return filename;
                }
        }

        closedir_checked(dir);

        upanic("No Makefile found in the current folder.")
}

__wur __nonnull() static bool is_sep(const_str arg) {
        return !strcmp(arg, "--");
}

_Noreturn int main(const int argc, Args argv) {
        store_usage(argv);
        assert(!strcmp(argv[0], "mk"));

        assert(!is_sep(argv[1]));
        const_str target = expand_target(argv[1]);

        if (!argv[2]) {
                const_str makefile = get_makefile(NULL);
                exldn("make", target, "-f", makefile);
        }

        if (!argv[3]) {
                assert(!is_sep(argv[2]));
                exldn("make", target, "-f", argv[2]);
        }

        const bool second_is_sep = is_sep(argv[2]);
        assert(second_is_sep ^ is_sep(argv[3]));

        const char *makefile;
        size_t start;

        if (second_is_sep) {
                makefile = get_makefile(NULL);
                start = 3;
        } else {
                makefile = get_makefile(argv[2]);
                start = 4;
        }

        make(target, makefile, argv + start, (size_t)argc - start);
}
