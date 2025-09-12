#include "lib.h"
#include "libexec.h"
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

static void usage(const_str arg0) {
        upanic("Too many arguments.\nUsage: %s <[user] "
               "<crate>|<url>> [-n name]",
               arg0);
}

static void parse_args(const int argc,
                       Args argv,
                       const char **user,
                       const char **crate,
                       const char **name) {

        if (argc < 2 || argc > 3) usage(argv[0]);

        int end = argc - 1;
        *name = NULL;

        for (int i = 1; i < argc; ++i) {
                if (strcmp(argv[i], "-n")) continue;

                end = i;
                if (argv[++i] == NULL) usage(argv[0]);

                *name = argv[i];

                if (argv[++i] != NULL) usage(argv[0]);
        }

        if (end == 1) {
                *user = NULL;
                *crate = argv[1];
        } else if (end == 2) {
                *user = argv[1];
                *crate = argv[2];
        } else {
                usage(argv[0]);
        }
}

static bool is_url(const_str v) {
        for (const char *ptr = v; *ptr != '\0'; ++ptr) {
                if (*ptr == '/' || *ptr == '.') return true;
        }
        return false;
}

static const_var_str get_repo(const_str url) {
        if (!strncmp(url, "http", 4)) return url;

        if (!strncmp(url, "git", 3) || !strncmp(url, "www", 3)) {
                const size_t len = (strlen(url) + sizeof("https://"));
                str repo = malloc(len * sizeof(char));
                str wrtr = stpcpy(repo, "https://");
                stpcpy(wrtr, url);
                return repo;
        }

        const size_t len = (strlen(url) + sizeof("https://github.com/"));
        str repo = malloc(len * sizeof(char));
        str wrtr = stpcpy(repo, "https://github.com/");
        stpcpy(wrtr, url);
        return repo;
}

int main(const int argc, Args argv) {
        store_usage(argv);

        const char *user;
        const char *crate;
        const char *name;
        parse_args(argc, argv, &user, &crate, &name);

        if (user != NULL) {
                char repo[100];
                sprintf(repo, "https://github.com/%s/%s", user, crate);
                exldn("cargo", "binstall", "--git", repo, "-y");
        }

        if (is_url(crate)) {
                const_str repo = get_repo(crate);
                exldn("cargo", "binstall", "--git", repo, "-y");
        }

        exldn("cargo", "cargo", "binstall", "-y", crate);
}
