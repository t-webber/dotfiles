#include "lib.h"
#include "libos.h"

#include <sys/stat.h>

#define c(scope, res)                                                          \
        case scope:                                                            \
                return res

#define e(scope, var) c(scope, getenv_checked(var))

static const char *__wur get_scope(const char scope) {

        switch (scope) {

                e('a', "APPS");
                e('b', "BLOB");
                e('d', "DEV");
                e('f', "FILES");
                e('h', "HOME");
                e('l', "LOGS");
                e('m', "CMD");
                e('o', "WORK");
                e('s', "SECRET");
                e('t', "DATA");
                e('w', "WASTE");
                e('x', "XDG_CONFIG_HOME");
                e('.', "DOT");

                c('e', "/etc");
                c('i', "/boot/efi");
                c('r', "/root");

        case 'c': {
                const_str cmd = getenv_checked("CMD");
                char *const path = malloc((strlen(cmd) + 5) * sizeof(char));
                sprintf(path, "%s/%s", cmd, "src");
                return path;
        }

        default:
                upanic("Invalid scope '%c'", scope);
        }
}

int main(const int argc, Args argv) {
        if (argc != 2) upanic("Missing arguments");
        store_usage(argv[0], "", false);

        const_str arg = argv[1];

        if (is_dir(arg)) {
                printf("%s\n", argv[1]);
                return 0;
        }

        const_str scope = get_scope(arg[0]);
        char path[512];
        snprintf(path, 512, "%s/%s", scope, arg + 1);

        if (is_dir(path)) {
                printf("%s\n", path);
                return 0;
        }

        upanic("No such directory: '%s'", path);
}
