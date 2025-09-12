#include "lib.h"
#include "libexec.h"
#include "libfiles.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

static void
ls(const_str filename, const bool is_verbose, const bool has_multiple) {
        if (!is_verbose) {
                forked_exldn("ls", "-A", filename);
                return;
        }

        if (has_multiple) {
                struct winsize w;
                ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
                const size_t cols = w.ws_col - 2 - strlen(filename);
                const size_t nb_equals = cols / 2;

                fputs("\033[35m", stdout);
                for (size_t i = 0; i < nb_equals; ++i) putchar('=');
                printf(" %s ", filename);
                for (size_t i = 0; i < nb_equals + (cols % 2); ++i)
                        putchar('=');
                puts("\033[0m");
        }

        forked_exldn("eza",
                     "-lA",
                     "--icons=always",
                     "--no-time",
                     "--no-user",
                     filename);
}

static void display_file(const_str filename, const bool is_verbose) {
        const size_t len = strlen(filename);
        const_str extension = get_filename_extension(filename, len);
        const bool is_not_alacrity
            = strcmp(getenv_checked("TERM"), "alacritty");

        exec_open_file(filename, extension, false, is_not_alacrity, is_verbose);
}

static void
display(const_str filename, const bool is_verbose, const bool has_multiple) {
        struct stat st;
        int x = stat(filename, &st);

        if (x != 0) epanic("Failed to read %s", filename);

        if (S_ISDIR(st.st_mode)) {
                ls(filename, is_verbose, has_multiple);
                return;
        }

        if (!S_ISREG(st.st_mode))
                upanic("Invalid file %s: type %d", filename, st.st_mode);

        pid_t pid = fork_checked();
        if (pid == 0) display_file(filename, is_verbose);
        fork_wait(pid);
}

int main(const int argc, const_str *const argv) {
        store_usage(argv);
        bool verbose = is_verbose(argv[0], "s", "v");

        if (argc == 1) {
                display(".", verbose, false);
                return 0;
        }

        if (argc == 2) {
                display(argv[1], verbose, argc > 2);
                return 0;
        }

        for (int file_nb = 1; file_nb < argc; ++file_nb) {
                display(argv[file_nb], verbose, argc > 2);
        }
}
