#include "lib.h"
#include "libexec.h"
#include "libfiles.h"
#include "libos.h"

#define usage                                                                  \
        upanic(MAGENTA "Usage:" RED " %s <" YELLOW "file" RED ">(" YELLOW      \
                       ".c" RED " [single|multi] || [<" YELLOW "folder" RED    \
                       ">])",                                                  \
               argv[0])

__wur static char *path(const_str file, const bool is_c, const_str folder) {
        char *const filepath = malloc(512);
        if (is_c) {
                sprintf(filepath, "%s/cmd/src/%s", getenv_checked("DOT"), file);
        } else if (folder == NULL) {
                sprintf(filepath, "%s/cmd/sh/%s", getenv_checked("DOT"), file);
        } else {
                sprintf(filepath, "%s/%s", folder, file);
        }
        if (is_file(filepath)) {}
        return filepath;
}

#define nl "\n"
#define nn "\n\n"
#define tl "\n    "

#define include(x) "#include \"" #x ".h\"" nn

#define normal                                                                 \
        include(lib) "int main(const int argc, Args argv) {" tl                \
                     "store_usage(argv[0], \"\", false);" tl "return 0;" nl    \
                     "}" nl

#define single include(libcmd) "make_single(\"g\", \"rg\", \"=.--hidden\", )" nl

#define multi                                                                  \
        include(libcmd) "const Cmd C[] = {" tl                                 \
                        "cmd(\"b\", \"build\", \"run\", )," nl "};" nl nl      \
                        "const Manual M[] = {" tl "{'?', \"--help\"}," tl      \
                        "{'C', \"--color=always\"}," tl "{'-', \"--\"}," nl    \
                        "};" nn "make_settings(S, C, M);" nl                   \
                        "make_main(S, \"zig\")" nl

#define sh "#!/bin/sh" nn "set -euo pipefail" nn

int main(const int argc, Args argv) {
        if (argc < 2 || argc > 3) usage;

        store_usage(argv[0], "", false);

        const_str ext = get_filename_extension(argv[1]);
        const bool is_c = !strcmp(ext, "c");
        const_str dest = path(argv[1], is_c, argv[2]);

        if (argc == 2) {
                if (is_c)
                        append_file(dest, normal);
                else {
                        append_file(dest, sh);
                        chmod_checked(dest, 0700);
                }
        } else if (!strcmp(argv[2], "single")) {
                if (!is_c) usage;
                append_file(dest, single);
        } else if (!strcmp(argv[2], "multi")) {
                if (!is_c) usage;
                append_file(dest, multi);
        } else if (is_c) {
                usage;
        } else {
                append_file(dest, sh);
                chmod_checked(dest, 0700);
        };

        const_str dot = getenv_checked("DOT");
        if (chdir(dot)) epanic("Failed to chdir to %s", dot);
        exldn(getenv_checked("EDITOR"), dest);
}
