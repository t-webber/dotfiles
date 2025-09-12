#include "lib.h"
#include "libexec.h"
#include <assert.h>
#include <dirent.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

__nonnull() static void copy_array(size_t *const idx,
                                   const char **const exec_args,
                                   Args cli_args,
                                   const size_t cli_args_len) {
        for (size_t i = 0; i < cli_args_len; ++i)
                exec_args[(*idx)++] = cli_args[i];
}

__nonnull() __wur static const
    char *single_work_file_runner(const_str file, const_str ext) {
        if (!strcmp(ext, "py")) return "python";

        if (!strcmp(ext, "sage")) return "sage";

        if (!strcmp(ext, "html")) return "brave";

        str line = malloc(sizeof(char *) * 64);
        FILE *fd = fopen_checked(file, "r");

        char *buf = fgets(line, 64, fd);
        fclose(fd);
        if (buf == NULL) epanic("Failed to read first line of %s", file);

        line[strcspn(line, "\n")] = '\0';

        if (!strncmp(line, "#!", 2)) return line + 2;

        return NULL;
}

__nonnull((1, 2)) static void run_file(const_str file,
                                       const_str ext,
                                       const_str *const argv,
                                       const size_t argv_len,
                                       const bool is_release) {

        if (!strcmp(ext, "c") || !strcmp(ext, "cc") || !strcmp(ext, "cpp")
            || !strcmp(ext, "rs")) {

                if (!strcmp(ext, "rs")) {
                        forked_exldn("rustc",
                                     file,
                                     "-o",
                                     "a.out",
                                     is_release ? "-C" : NULL,
                                     "opt-level=3");
                } else {
                        const_str cmd = strcmp(ext, "c") == 0 ? "cc" : "c++";
                        const_str opt = is_release ? "-Ofast" : "-O0";
                        const_str flag = is_release ? "-flto" : "-g";
                        forked_exldn(cmd, opt, flag, file, "-o", "a.out");
                }

                const size_t len = argv_len + 2;
                const char **const args = malloc(sizeof(char *) * len);

                printf("Running a.out\n");
                size_t idx = 0;

                args[idx++] = "./a.out";
                copy_array(&idx, args, argv, argv_len);
                args[idx++] = NULL;
                exvd(args);
        }

        if (!strcmp(ext, "go")) {
                const size_t len = argv_len + 4;
                const char **const args = malloc(sizeof(char *) * len);
                size_t idx = 0;

                args[idx++] = "go";
                args[idx++] = "run";
                args[idx++] = file;
                copy_array(&idx, args, argv, argv_len);
                args[idx++] = NULL;
                exvd(args);
        }

        const_str prog = single_work_file_runner(file, ext);
        if (prog == NULL) return;

        const size_t len = argv_len + 3;
        const char **const args = malloc(sizeof(char *) * len);
        size_t idx = 0;

        args[idx++] = prog;
        args[idx++] = file;
        copy_array(&idx, args, argv, argv_len);
        args[idx++] = NULL;
        exvd(args);
}

__wur static int run_folder(const bool is_release) {
        char cwd[128];
        char *buf = getcwd(cwd, 128);
        if (buf == NULL) {
                epanic("Failed to get current working "
                       "directory");
        }

        DIR *dirp = opendir_checked(cwd);
        struct dirent *entry;
        while ((entry = readdir(dirp))) {
                const_str name = entry->d_name;

                if (!strcmp(name, "Cargo.toml"))
                        exldn("cargo", "run", is_release ? "--release" : NULL);

                if (!strcmp(name, "go.mod")) exldn("go", "run", ".");

                if (!strcmp(name, "Makefile")) upanic("Use m");

                if (!strcmp(name, "package.json")) {
                        forked_exldn("bun", "i");
                        if (is_release) {
                                forked_exldn("bun", "run", "build");
                                exldn("bun", "run", "start");
                        } else {
                                exldn("bun", "run", "dev");
                        }
                }
        }
        closedir(dirp);

        dirp = opendir_checked(cwd);
        while ((entry = readdir(dirp))) {
                const_str name = entry->d_name;

                const_str last_full_stop = strrchr(name, '.');
                if (!last_full_stop) continue;

                const_str ext = last_full_stop + 1;
                if (*ext < 'a' || *ext > 'z') continue;
                printf("%s\n", last_full_stop);

                run_file(name, ext, NULL, 0, is_release);
        }
        closedir(dirp);

        upanic("No runner found in the current folder");
}

__wur __nonnull() __attribute_pure__ static const
    char *as_extension(const_str file) {
        const char *ptr = file;
        for (; *ptr != '\0'; ++ptr)
                if (*ptr == '.') return ptr + 1;
        return ptr;
}

int main(const int argc, Args argv) {
        store_usage(argv);

        const bool is_release = is_verbose(argv[0], "run", "runr");

        if (argc == 1) return run_folder(is_release);

        const_str file = argv[1];
        const_str ext = as_extension(file);

        run_file(file, ext, argv + 2, (size_t)argc - 2, is_release);
        upanic("No runner for file %s", file);
}
