#include "lib.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

const String HTTP = NEW_STRING("http");
const String ME = NEW_STRING("t-webber");
const String TE = NEW_STRING("telecom-etude");
const String GITHUB = NEW_STRING("https://github.com/");

static void git_clone(const char *const url, char *const alloc_url,
                      bool finish) {
        printf("* [%s]\n", url);

        pid_t pid = fork();
        if (pid == 0) {
                execl("/usr/bin/git", "git", "clone", url, NULL);
                panic("Failed to spawn subprocess for git clone.\n");
        } else {
                int status;
                waitpid(pid, &status, 0);

                if (status == 0 || finish) {
                        if (alloc_url != NULL)
                                free(alloc_url);
                        exit(status);
                }
        }
}

static void find_repo(const char *const repo_name) {
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

                char *const url = malloc(128 * sizeof(char));
                const ssize_t len = read(pipefd[0], url, 127);
                if (len < 1)
                        panic("Unexpectedly found empty remote url.\n");

                url[len - 1] = '\0';
                git_clone(url, url, true);
        }
}

int main(const int argc, const char *const *const argv) {
        if (argc == 1)
                panic("Missing argument.\n");

        if (argc > 3)
                panic("Too many arguments.\n");

        if (argc == 3) {
                const char *user = argv[1];
                if (!strcmp(user, "te"))
                        user = "telecom-etude";

                const char *const repo = argv[2];
                const size_t len =
                    GITHUB.length + strlen(user) + 1 + strlen(repo);
                char *const url = malloc((len + 1) * sizeof(char));
                sprintf(url, "%s%s/%s", GITHUB.value, user, repo);
                git_clone(url, url, true);
        }

        const char *const arg = argv[1];

        const char *slash_positon = NULL;
        size_t arg_len;
        bool arg_has_slash = has_slash(arg, &arg_len, &slash_positon);

        if (access(slash_positon, F_OK) == 0)
                panic("File %s already exists.\n", slash_positon);

        printf("Ok\n");

        if (!strncmp(HTTP.value, arg, HTTP.length))
                git_clone(arg, NULL, true);

        if (arg_has_slash) {
                const size_t len = GITHUB.length + arg_len;
                char *const url = malloc((len + 1) * sizeof(char));
                sprintf(url, "%s%s", GITHUB.value, arg);
                git_clone(url, url, false);
                free(url);
        }

        const size_t url_cap = GITHUB.length + TE.length + 1 + arg_len + 1;
        char *const url = malloc(url_cap * sizeof(char));
        char *const github = stpcpy(url, GITHUB.value);

        sprintf(github, "%s/%s", ME.value, arg);
        git_clone(url, url, false);

        sprintf(github, "%s/%s", TE.value, arg);
        git_clone(url, url, false);

        free(url);

        find_repo(arg);

        panic("unreachable");
}
