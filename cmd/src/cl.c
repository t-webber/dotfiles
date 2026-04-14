#include "lib.h"
#include "libexec.h"
#include "libvec.h"

#include <string.h>

int main(const int argc, Args argv) {
        store_usage(argv[0], "", false);

        if (argc == 1) upanic("Usage: %s <v|ast|file>", argv[0]);

        if (!strcmp(argv[1], "v")) { exldn("clang++", "--version"); }

        Vec cmd = new_v();
        push_v(&cmd, "clang++");
        int start = 1;

        if (!strcmp(argv[1], "ast")) {
                start = 2;

                push_v(&cmd, "-fsyntax-only");
                push_v(&cmd, "-Xclang");
                push_v(&cmd, "-ast-dump");
        }

        for (int i = start; i < argc; ++i) { push_v(&cmd, argv[i]); }

        push_v(&cmd, NULL);

        exvd(cmd.data);
}
