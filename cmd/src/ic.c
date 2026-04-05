#include "lib.h"
#include "libexec.h"
#include "libvec.h"
#include <assert.h>

__nonnull() static void install_gh_s(void) {
        read_simple_exldn(126, buffer, "gh", "s", "--version");
        if (starts_with_const(buffer, "unknown command ")) {
                printf("Installing gh-s...\n");
                forked_exldn("gh", "ext", "install", "gennaro-tedesco/gh-s");
                install_gh_s();
        } else if (!starts_with_const(buffer, "gh-s "))
                upanic("gh s --version returned invalid ouput");
}

_Noreturn __nonnull() static void run_gh_s(const int argc, Args argv) {
        Vec cmd = new_vec();
        push(&cmd, "gh");
        push(&cmd, "s");
        for (int i = 1; i < argc; ++i) push(&cmd, argv[i]);
        read_simple_exvd(512, buf, cmd.data);
        buf[strlen(buf) - 1] = '\0';
        exldn("git", "clone", buf);
}

__wur __nonnull() static bool is_url_valid(const_str url) {
        read_simple_exldn(126,
                          buf,
                          "curl",
                          "-o",
                          "/dev/null",
                          "-s",
                          "-w",
                          "%{http_code}",
                          "-L",
                          url);
        if (!strcmp("200", buf)) return true;
        if (!strcmp("404", buf)) return false;
        upanic("Invalid status code for url %s: %s\n", url, buf);
}

__nonnull() static void try_clone(const_str prefix, const_str name) {
        char repo[512];
        char *ptr = repo;
        ptr = stpcpy(ptr, "https://");
        ptr = stpcpy(ptr, prefix);
        ptr = stpcpy(ptr, name);
        printf("Trying %s...\n", repo);
        if (is_url_valid(repo)) exldn("git", "clone", repo);
}

#define url_len 3
const_str urls[url_len] = {
    "github.com/t-webber/",
    "github.com/rust-lang/",
    "git.suckless.org/",
};

int main(const int argc, Args argv) {
        store_usage(argv[0], "", false);
        assert(argc > 1);
        if (argc == 2) {
                if (strchr(argv[1], '/')) try_clone("github.com/", argv[1]);
                for (int i = 0; i < url_len; ++i) try_clone(urls[i], argv[1]);
        }
        printf("Trying with gh s...\n");
        install_gh_s();
        run_gh_s(argc, argv);
}
