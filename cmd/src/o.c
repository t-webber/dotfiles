#include "lib.h"
#include "libexec.h"
#include "libfiles.h"
#include "libos.h"
#include <stdbool.h>
#include <sys/stat.h>
#include <unistd.h>

static __nonnull() __wur const
    char *argv_one_filename(const int argc, const_str *const argv) {
        if (argc == 1)
                return ".";
        else if (argc == 2)
                return argv[1];
        else
                upanic("Too many arguments. Usage: %s [<filename>]", argv[0]);
}

#define browsers_len 2
const_str browsers[browsers_len]
    = {"/bin/brave", "/Applications/Chromium.app/Contents/MacOS/Chromium"};

int main(const int argc, const_str *const argv) {
        store_usage(argv[0], "", false);
        const char *filename = argv_one_filename(argc, argv);

        struct stat st;
        int x = stat(filename, &st);

        if (x != 0) {
                char url[256];
                if (starts_with_const(filename, "http")) {
                        stpcpy(url, filename);
                } else if (strchr(filename, '.') != NULL) {
                        stpcpy(stpcpy(url, "http://"), filename);
                } else {
                        char *wrt
                            = stpcpy(url, "https://www.google.com/search?q=");
                        for (const char *rdr = filename; *rdr; ++rdr) {
                                if (*rdr == ' ') {
                                        *wrt++ = '%';
                                        *wrt++ = '2';
                                        *wrt++ = '0';
                                } else {
                                        *wrt++ = *rdr;
                                }
                        }
                }

                for (int i = 0; i < browsers_len; ++i)
                        if (is_file(browsers[i])) {
                                if (fork_checked() == 0)
                                        exldn(browsers[i], url);
                                return 0;
                        }

                upanic("No known browser, please add it");
        }

        if (S_ISDIR(st.st_mode)) { exldn("nvim", filename, NULL); }

        if (!S_ISREG(st.st_mode))
                upanic("Invalid file %s: type %o", filename, st.st_mode);

        exec_open_file(filename, DISPLAY_OPEN);
}
