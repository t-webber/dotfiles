#include "lib.h" // contains the panic macro that prints and exits, and Argv typedefed to const char*const*const
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define url_max_len 128
#define cmd_max_len 256

/// Panics with a message and an explanation about how to use this program.
static void usage(const char *const message, const char *const program) {
        panic("%s\nUsage: %s [te/|rs/|?/|<user>/]<repo-name> [-- [git clone "
              "opts]]\n",
              message, program);
}

/// Parses argv to separate them between icl arguments (passed to this program)
/// or the git arguments (passed to git clone).
///
/// Git arguments come after `--`.
static void separate_icl_git_args(const int argc, Argv argv,
                                  size_t *const icl_len,
                                  const char **const icl_args,
                                  const char **const git_args) {
        bool found_git_args = false;
        *icl_len = 0;
        size_t git_len = 0;
        git_args[git_len++] = "git";
        git_args[git_len++] = "clone";
        ++git_len; // url

        for (int i = 1; i < argc; ++i) {
                if (!strcmp(argv[i], "--")) {
                        found_git_args = true;
                } else if (found_git_args) {
                        git_args[git_len++] = argv[i];
                } else {
                        icl_args[(*icl_len)++] = argv[i];
                }
        }

        git_args[git_len] = NULL;
}

/// Check if the url exists, and if it does, clones it.
static void try_clone_url(const char *const url, const char **const git_args) {
        printf("> Trying %s.\n", url);

        char cmd[cmd_max_len];
        sprintf(cmd,
                "curl -o /dev/null -s -w \"%%{http_code}\" -L %s | grep -q 200",
                url);
        if (system(cmd))
                return;

        printf("> Exists. Cloning.\n");
        git_args[2] = url;

        for (size_t i = 0; git_args[i] != NULL; ++i)
                printf("# %s\n", git_args[i]);

        printf("> Cloning...\n");

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
        execv("/bin/git", (char **const)git_args);
#pragma GCC diagnostic pop
        fprintf(stderr, "execv failed: %s\n", strerror(errno));
        panic("!");
}

/// Uses gh s to search a repository with the given name. It gets the selected
/// URL, and clones it.
static void find_and_clone(const char *const repo_name,
                           const char **const icl_args) {
        int pipefd[2];
        pipe(pipefd);
        pid_t pid = fork();
        if (pid == 0) {
                dup2(pipefd[1], STDOUT_FILENO);
                execl("/usr/bin/gh", "gh", "s", repo_name, NULL);
                panic("Failed to spawn subprocess for gh s.\n");
        } else {
                int status;
                waitpid(pid, &status, 0);

                if (status)
                        panic("No repository named %s found.\n", repo_name);

                char *const url = malloc(url_max_len * sizeof(char));
                const ssize_t len = read(pipefd[0], url, url_max_len - 1);
                if (len <= 1)
                        panic("Unexpectedly found empty remote url.\n");

                url[len - 1] = '\0';
                try_clone_url(url, icl_args);
                free(url);
                panic("Failed to select a repository.\n");
        }
}

#define remotes_len 3
/// List of remotes searched by default if no remote is specified
const char *const remotes[remotes_len] = {"github.com", "git.rezel.net",
                                          "git.suckless.org"};
#define users_len 3
/// List of users searched by default if no user is specified
const char *const users[users_len] = {
    "t-webber",
    "telecom-etude",
    "rust-lang",
};

/// Try to clone a repo with the given user name and repo name.
static void try_clone_with_user(const char *user, const char *repo,
                                const char **const git_args) {
        if (!strcmp("te", user))
                user = "telecom-etude";

        if (!strcmp("rs", user))
                user = "rust-lang";

        char url[url_max_len];
        for (size_t i = 0; i < remotes_len; ++i) {
                sprintf(url, "https://%s/%s/%s", remotes[i], user, repo);
                try_clone_url(url, git_args);
        }
}

/// Try and find a URL that corresponds to the user input, and
/// clone the git repository corresponding to that URL.
static void parse_and_clone(const size_t icl_len, Argv icl_args,
                            const char **const git_args) {

        if (icl_len == 2) {
                try_clone_with_user(icl_args[0], icl_args[1], git_args);
                panic("Failed to find a git repository with username %s and "
                      "name %s.\n",
                      icl_args[0], icl_args[1]);
        }

        const char *const icl_arg = icl_args[0];

        if (!strncmp("http", icl_arg, 4)) {
                try_clone_url(icl_arg, git_args);
                panic("%s isn't a valid git "
                      "repository.\n",
                      icl_arg);
        }

        const char *const slash = strchr(icl_arg, '/');

        if (slash != NULL) {
                char user[32];
                strncpy(user, icl_arg, (unsigned long)(slash - icl_arg));
                user[slash - icl_arg] = '\0';
                try_clone_with_user(user, slash + 1, git_args);
                panic("Found no remote with repo %s/%s.\n", user, icl_arg);
        }

        for (size_t i = 0; i < users_len; ++i)
                try_clone_with_user(users[i], icl_arg, git_args);

        find_and_clone(icl_arg, git_args);
        panic("Found no repo named %s with common users.\n", icl_arg);
}

int main(const int argc, Argv argv) {
        if (argc <= 1 || !strcmp(argv[1], "--"))
                usage("Missing arguments: expected 1 or 2, found "
                      "0.",
                      argv[0]);

        const char **const icl_args =
            malloc(((unsigned long)argc + 1) * sizeof(char *));
        const char **const git_args =
            malloc(((unsigned long)argc + 3) * sizeof(char *));
        size_t icl_len;

        separate_icl_git_args(argc, argv, &icl_len, icl_args, git_args);

        if (icl_len > 2)
                usage("Too many arguments: expected 1 or 2.", argv[0]);

        parse_and_clone(icl_len, icl_args, git_args);

        free(icl_args);
        free(git_args);
}
