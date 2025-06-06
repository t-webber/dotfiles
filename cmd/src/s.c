#include "lib.h"
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

static int display_file(const char *const filename, const bool is_verbose) {
        struct stat st;
        int x = stat(filename, &st);

        if (x != 0)
                panic("Failed to read %s: exited with code %d.\n", filename,
                      errno);

        if (S_ISDIR(st.st_mode)) {
                if (!is_verbose)
                        return execlp("ls", "ls", "-A", filename, NULL);

                struct winsize w;
                ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
                const size_t cols = w.ws_col - 2 - strlen(filename);
                const size_t nb_equals = cols / 2;

                fputs("\033[35m", stdout);
                for (size_t i = 0; i < nb_equals; ++i)
                        putchar('=');
                printf(" %s ", filename);
                for (size_t i = 0; i < nb_equals + (cols % 2); ++i)
                        putchar('=');
                puts("\033[0m");

                return execlp("eza", "eza", "-Z@lA", "--icons=always", "--git",
                              filename, NULL);
        }

        if (!S_ISREG(st.st_mode))
                panic("Invalid file %s: type %d.", filename, st.st_mode);

        const size_t len = strlen(filename);
        const char *const extension = get_filename_extension(filename, len);

        if (is_valid_extension(TIMG_EXTENSIONS, extension, TIMG_EXTENSIONS_LEN))
                return execlp("timg", "timg", filename, NULL);

        if (!strcmp(extension, "pdf") && strcmp(getenv("TERM"), "alacritty"))
                return execlp("tdf", "tdf", filename, NULL);

        if (is_valid_extension(BRAVE_EXTENSIONS, extension,
                               BRAVE_EXTENSIONS_LEN))
                return execlp("brave", "brave", filename, NULL);

        const char *const program = is_verbose ? "bat" : "cat";
        return execlp(program, program, filename, NULL);
}

static int safe_display_file(const char *const filename,
                             const bool is_verbose) {
        int res = display_file(filename, is_verbose);
        panic("Failed to read %s: exec exited with code %d: %s.\n", filename,
              res, strerror(errno));
}

int main(const int argc, const char *const *const argv) {
        fputs("\033[2J\033[H", stdout);
        fflush(stdout);

        bool verbose = is_verbose(argv[0], "s", "sv");

        if (argc == 1)
                return safe_display_file(".", verbose);

        for (int file_nb = 1; file_nb < argc; ++file_nb) {

                pid_t pid = fork();

                if (pid < 0)
                        panic("Failed to fork the %d-th time.\n", file_nb);

                if (pid == 0)
                        return safe_display_file(argv[file_nb], verbose);

                int status;
                waitpid(pid, &status, 0);
                if (!WIFEXITED(status))
                        panic("Child process failed with status %d.\n", status);
        }
}
