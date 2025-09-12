#include "lib.h"
#include "libexec.h"
#include "libfiles.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main(const int argc, const_str *const argv) {
        store_usage(argv);
        const char *filename = argv_one_filename(argc, argv);

        struct stat st;
        int x = stat(filename, &st);

        if (x != 0) epanic("Failed to read %s", filename);

        if (S_ISDIR(st.st_mode)) { exldn("nvim", filename, NULL); }

        if (!S_ISREG(st.st_mode))
                upanic("Invalid file %s: type %d.", filename, st.st_mode);

        const size_t len = strlen(filename);
        const_str extension = get_filename_extension(filename, len);
        const bool is_not_alacritty
            = strcmp(getenv_checked("TERM"), "alacritty");
        return exec_open_file(filename,
                              extension,
                              true,
                              is_not_alacritty,
                              false);
}
