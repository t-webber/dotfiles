#include "lib.h"
#include <assert.h>
#include <unistd.h>

// ci email
// ci eza
// ci repo

static int install_git(const char *const repo, const char *const name) {
        printf("Installing repo %s with name %s...\n", repo, name);
        int res =
            execlp("cargo", "cargo", "install", "--git", repo, name, NULL);
        panic("Failed to execute: exit code %d.\n", res);
}

int main(const int argc, const char *const *const argv) {

        if (argc <= 1)
                panic("Missing argument.\n");

        if (argc == 3) {
                char repo[100];
                sprintf(repo, "https://github.com/%s/%s", argv[1], argv[2]);
                const size_t len = strlen(repo);
                assert(repo[len] == '\0');
                return install_git(repo, argv[2]);
        }

        const char *const arg = argv[1];

        const char *slash_position = NULL;
        size_t arg_len;
        bool arg_has_slash = has_slash(arg, &arg_len, &slash_position);

        if (arg_has_slash) {
                if (!strncmp("https://", arg, 8))
                        return install_git(arg, slash_position);

                char repo[100];
                sprintf(repo, "https://github.com/%s", arg);
                return install_git(repo, slash_position);
        }

        int res = execlp("cargo", "cargo", "binstall", "-y", arg, NULL);
        panic("Failed to execute: exit code %d.\n", res);
}
