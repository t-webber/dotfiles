#include "lib.h"
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main(const int argc, const char *const *const argv) {

        const char *filename = argv_one_filename(argc, argv);
        bool verbose = is_verbose(argv[0], "s", "sv");

        struct stat st;
        int x = stat(filename, &st);

        if (x != 0)
                panic("Failed to read %s: exited with code %d.\n", filename,
                      errno);

        if (S_ISDIR(st.st_mode)) {
                if (!verbose)
                        return execlp("ls", "ls", "-A", filename, NULL);

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

        const char *const program = verbose ? "bat" : "cat";
        return execlp(program, program, filename, NULL);
}
