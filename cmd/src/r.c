#include "lib.h"
#include <dirent.h>
#include <sys/wait.h>
#include <unistd.h>

#define exl(cmd, ...)                                                          \
        {                                                                      \
                execlp(cmd, __VA_ARGS__, NULL);                                \
                panic("Failed to execute %s.\n", cmd)                          \
        }

static void try_run_ext(const char *const file, const char *const file_ext,
                        const char *const goal_ext,
                        const char *const goal_cmd) {
        if (!strcmp(file_ext, goal_ext))
                exl(goal_cmd, goal_cmd, file)
}

static int exv(const char *const *const argv) {
        execvp(argv[0], (char *const *const)argv);
        panic("Failed to execute %s\n", argv[0]);
}

static int run_successive(const char *const *const args) {
        pid_t pid = fork();

        if (pid == 0)
                exv(args);

        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                const char *const *ptr = args;
                while (*ptr != NULL)
                        ++ptr;
                exv(++ptr);
        }
        panic("Unexpected panic on pid %d and status %d.\n", pid, status);
}

static int run_file(const char *const file, const char *const ext,
                    const char *const *const argv, const int argv_len,
                    const bool is_verbose) {

        if (!strcmp(ext, "c") || !strcmp(ext, "cc") || !strcmp(ext, "cpp")) {
                const size_t len = 8 + (size_t)argv_len;
                const char **const args = malloc(sizeof(char *) * len);
                size_t index = 0;

                args[index++] = strcmp(ext, "c") == 0 ? "cc" : "c++";
                args[index++] = is_verbose ? "-g" : "-O3";
                args[index++] = file;
                args[index++] = "-o";
                args[index++] = "a.out";
                args[index++] = NULL;
                args[index++] = "./a.out";

                for (int i = 0; i < argv_len; ++i)
                        args[index++] = argv[i];

                args[index++] = NULL;

                run_successive(args);
        }

        if (!strcmp(ext, "rs")) {
                const size_t len = (size_t)argv_len + 7 + (is_verbose ? 0 : 2);
                const char **const args = malloc(sizeof(char *) * len);
                size_t index = 0;

                args[index++] = "rustc";
                if (!is_verbose) {
                        args[index++] = "-C";
                        args[index++] = "opt-level=3";
                }
                args[index++] = file;
                args[index++] = "-o";
                args[index++] = "a.out";
                args[index++] = NULL;
                args[index++] = "./a.out";

                for (int i = 0; i < argv_len; ++i)
                        args[index++] = argv[i];

                args[index++] = NULL;

                run_successive(args);
        }

        if (!strcmp(ext, "py")) {
                const size_t len = (size_t)argv_len + 3;
                const char **const args = malloc(sizeof(char *) * len);
                size_t index = 0;

                args[index++] = "python3";
                args[index++] = file;

                for (int i = 0; i < argv_len; ++i)
                        args[index++] = argv[i];

                args[index++] = NULL;

                run_successive(args);
        }

        try_run_ext(file, ext, "py", "python");
        try_run_ext(file, ext, "sage", "sage");
        try_run_ext(file, ext, "html", "brave");

        FILE *fd = fopen(file, "r");
        if (fd == NULL)
                panic("Failed to open %s.\n", file);

        char line[64];
        char *buf = fgets(line, 64, fd);
        fclose(fd);
        if (buf == NULL)
                panic("Failed to read first line of %s.\n", file);

        line[strcspn(line, "\n")] = '\0';
        exl(line + 2, line + 2, file);
}

static int run_folder(void) {

        char cwd[128];
        char *buf = getcwd(cwd, 128);
        if (buf == NULL)
                panic("Failed to get current working directory\n");

        DIR *dirp = opendir(cwd);
        struct dirent *entry;
        while ((entry = readdir(dirp))) {
                if (!strcmp(entry->d_name, "Cargo.toml"))
                        exl("cargo", "cargo", "run");

                if (!strcmp(entry->d_name, "package.json"))
                        return run_successive((const char *const[]){
                            "bun", "i", NULL, "bun", "run", "dev"});

                if (!strcmp(entry->d_name, "Makefile"))
                        exl("make", "make");
        }

        panic("Found no runner in the current folder.\n");
}

int main(const int argc, const char *const *const argv) {
        store_usage(argv);

        const bool verbose = is_verbose(argv[0], "r", "rv");

        if (argc == 1)
                return run_folder();

        const char *const file = argv[1];
        const char *extension = file;

        for (const char *ptr = file; *ptr != '\0'; ++ptr)
                if (*ptr == '.')
                        extension = ptr + 1;

        return run_file(file, extension, argv + 2, argc - 2, verbose);
}
