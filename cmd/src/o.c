#include "lib.h"
#include "libexec.h"
#include "libfiles.h"
#include <stdbool.h>
#include <sys/stat.h>
#include <unistd.h>

int main(const int argc, const_str *const argv) {
        store_usage(argv[0], "", false);
        const char *filename = argv_one_filename(argc, argv);

        struct stat st;
        int x = stat(filename, &st);

        if (x != 0) epanic("Failed to read %s", filename);

        if (S_ISDIR(st.st_mode)) { exldn("nvim", filename, NULL); }

        if (!S_ISREG(st.st_mode))
                upanic("Invalid file %s: type %o", filename, st.st_mode);

        exec_open_file(filename, DISPLAY_OPEN);
}
