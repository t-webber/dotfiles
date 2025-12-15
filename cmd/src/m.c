#include "lib.h"
#include "libexec.h"
#include "zlib/zlib.h"
#include <assert.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static bool open_man_page(const_str full_path, bool only_c_pages) {
        gzFile fd = gzopen(full_path, "r");
        char line[128];

        gzgets(fd, line, 128);
        if (!strncmp(".so ", line, 4)) {
                gzclose(fd);
                line[strlen(line) - 1] = '\0';
                forked_exldn("m", line + 4);
                return true;
        }

        bool display = !only_c_pages;

        if (!display) {
                while (gzgets(fd, line, 128)) {
                        if (strstr(line, "#include <")) {
                                display = true;
                                break;
                        }
                }
        }

        gzclose(fd);

        if (display) { forked_exldn("man", full_path); }

        return display;
}

static size_t get_man_pages_in_folder(bool only_c_pages,
                                      const_str page_name,
                                      const size_t page_name_len,
                                      const_str folder_name) {
        const size_t folder_name_len = strlen(folder_name);
        size_t len = 0;

        char path[32];

        sprintf(path, "/usr/share/man/%s", folder_name);

        DIR *d = opendir(path);
        if (d == NULL) epanic("Failed to open %s", path);

        struct dirent *file;
        while ((file = readdir(d))) {
                if (file->d_type != DT_REG && file->d_type != DT_LNK) continue;

                char *pos = strstr(file->d_name, page_name);
                if (pos == NULL) continue;

                const size_t file_name_len = strlen(file->d_name);
                const_str end = pos + page_name_len;

                if (pos != file->d_name || *end != '.') continue;

                str full_path
                    = (str)malloc(sizeof(char)
                                  * (sizeof("/usr/share/man/") + folder_name_len
                                     + 1 + file_name_len + 1));
                char *writer = stpcpy(full_path, "/usr/share/man/");
                writer = stpcpy(writer, folder_name);
                *writer++ = '/';
                writer = stpcpy(writer, file->d_name);

                if (open_man_page(full_path, only_c_pages)) ++len;
        }

        closedir(d);
        return len;
}

static size_t get_man_pages(const bool only_c_pages, const_str page_name) {
        const size_t page_name_len = strlen(page_name);

        DIR *d = opendir_checked("/usr/share/man");

        struct dirent *file;
        size_t len = 0;

        while ((file = readdir(d))) {
                if (!strcmp(file->d_name, ".") || !strcmp(file->d_name, "..")
                    || file->d_type != DT_DIR)
                        continue;

                len += get_man_pages_in_folder(only_c_pages,
                                               page_name,
                                               page_name_len,
                                               file->d_name);
        }

        closedir(d);
        return len;
}

static void parse_argv(int *const argc,
                       const_str *const argv,
                       bool *const c_man,
                       bool *const git,
                       bool *const clear,
                       bool *const mount) {
        *git = !strcmp(argv[0], "mg");
        *c_man = !strcmp(argv[0], "mc");

        if (!c_man && !git && strcmp(argv[0], "m"))
                upanic("Erroneous program name. Should be m, mc or mg, but "
                       "used %s.\n",
                       argv[0]);

        *clear = strcmp(argv[*argc - 1], "!");

        if (!clear) --*argc;

        *mount = *argc > 1 && argv[1][0] == '/';

        bool invalid_args = (*argc == 2 && !strcmp(argv[1], "?"))
                            || ((*argc > 4 || *argc < 3) && *mount)
                            || (*argc > 3 && !*mount) || (*c_man && *argc != 2);
        if (invalid_args) {
                printf("Usage: %s (? | [(<target>|<alias>) "
                       "[<Makefile>]] | "
                       "<page-name> | (<device> <path>)) "
                       "[!]\n\n",
                       argv[0]);
                exit(1);
        }
}

static int git_man_page(const int argc, Args argv) {
        assert(argc == 2);
        str page_name = (char *)malloc(30 * sizeof(char));
        sprintf(page_name, "git-%s", argv[1]);
        assert(open_man_page(page_name, false));
        return 0;
}

int main(const int argc, Args argv) {
        store_usage(argv);
        bool c_man;
        bool git;
        bool clear;
        bool is_mount;
        int normalised_argc = argc;
        parse_argv(&normalised_argc, argv, &c_man, &git, &clear, &is_mount);

        if (git) {
                if (clear) { system("clear"); }

                return git_man_page(argc, argv);
        }

        if (clear) system("clear");

        const size_t nb = get_man_pages(c_man, argv[1]);
        if (nb == 0) upanic("No man page found\n");

        return 0;
}
