#include "lib.h"
#include "libexec.h"
#include "libvec.h"
#include <assert.h>

__nonnull() static void install_gh_s(void) {
        read_simple_exldn_stderr(126, buffer, "gh", "s", "--version");
        if (starts_with_const(buffer, "unknown command ")) {
                printf("Installing gh-s...\n");
                forked_exldn("gh", "ext", "install", "gennaro-tedesco/gh-s");
                install_gh_s();
        } else if (!starts_with_const(buffer, "gh-s "))
                upanic("gh s --version returned invalid ouput");
}

_Noreturn __nonnull() static void run_gh_s(const int argc, Args argv) {
        Vec cmd = new_v();
        push_v(&cmd, "gh");
        push_v(&cmd, "s");
        for (int i = 1; i < argc; ++i) push_v(&cmd, argv[i]);
        read_simple_exvd(512, buf, cmd.data);
        buf[strlen(buf) - 1] = '\0';
        exldn("git", "clone", buf);
}

int main(const int argc, Args argv) {
        store_usage(argv[0], "", false);
        install_gh_s();
        run_gh_s(argc, argv);
}
