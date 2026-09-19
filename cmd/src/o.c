#include "lib.h"
#include "libexec.h"
#include "libfiles.h"
#include "libos.h"
#include <stdbool.h>
#include <sys/stat.h>
#include <unistd.h>

#define browsers_len 3
const_str browsers[browsers_len] = {
    "/bin/brave",
    "/Applications/Chromium.app/Contents/MacOS/Chromium",
    "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome",
};

_Noreturn static void open_file(const char *filename) {
        struct stat st;
        int x = stat(filename, &st);
        const_str ext = get_filename_extension(filename);

        if (x != 0 || !strcmp(ext, "html")) {

                char new[64];
                if (starts_with_const(filename, "l:")) {
                        char *w = stpcpy(new, "http://localhost:");
                        stpcpy(w, filename + 2);
                        filename = new;
                }

                if (!strcmp(filename, ".")) upanic("<unreachable but needed by gcc>");

                char url[256];
                if (starts_with_const(filename, "http") || x == 0) {
                        stpcpy(url, filename);
                } else if (strchr(filename, '.') != NULL) {
                        stpcpy(stpcpy(url, "http://"), filename);
                } else {
                        char *wrt = stpcpy(url, "https://www.google.com/search?q=");
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
                                if (fork_checked() == 0) exldn(browsers[i], url);
                                exit(0);
                        }

                upanic("No known browser, please add it");
        }

        if (S_ISDIR(st.st_mode)) {
                const size_t len = strlen(filename);
                if (len > 5) {
                        const_str suffix = filename + len - sizeof(".app/") + 1;
                        if (!strcmp(suffix, ".app/") || !strcmp(suffix + 1, ".app"))
                                exldn("/usr/bin/open", filename);
                }
                exldn("nvim", filename);
        }

        if (!S_ISREG(st.st_mode)) upanic("Invalid file %s: type %o", filename, st.st_mode);

        exec_open_file(filename, DISPLAY_OPEN);
}

int main(const int argc, const_str *const argv) {
        for (int i = 1; i < argc; ++i)
                if (!fork_and_wait()) open_file(argv[i]);
}
