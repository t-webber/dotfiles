#include "lib.h"
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int open_file(const char *const filename) {
        const size_t len = strlen(filename);
        const char *const extension = get_filename_extension(filename, len);
        const bool is_not_alacritty = strcmp(getenv("TERM"), "alacritty");

        if (has_timg_support(extension))
                if (is_not_alacritty)
                        return execlp("timg", "timg", filename, NULL);

        if (!strcmp(extension, "pdf"))
                if (is_not_alacritty)
                        return execlp("tdf", "tdf", filename, NULL);

        if (has_brave_support(extension) || has_timg_support(extension))
                return execlp("brave", "brave", filename, NULL);

        return execlp("nvim", "nvim", filename, NULL);
}

int main(const int argc, const char *const *const argv) {

        const char *filename = argv_one_filename(argc, argv);

        struct stat st;
        int x = stat(filename, &st);

        if (x != 0)
                panic("Failed to read %s: exited with code %d.\n", filename,
                      errno);

        if (S_ISDIR(st.st_mode)) {
                return execlp("nvim", "nvim", filename, NULL);
        }

        if (!S_ISREG(st.st_mode))
                panic("Invalid file %s: type %d.", filename, st.st_mode);

        return open_file(filename);
}
