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

static bool
line_has_target(const_str line, const_str target, const size_t target_len) {
        if (!strncmp(line, "%:", 2)) return true;

        if (line[0] == '\0' || line[0] == '\t' || line[0] == ' '
            || !strchr(line, ':'))
                return false;

        const char *start = line;
        const size_t line_len = strlen(line);

        while (start + target_len < line + line_len) {
                if (*start == ' ') {
                        ++start;
                        continue;
                };

                if (!strncmp(target, start, target_len)
                    && (start[target_len] == ':' || start[target_len] == ' ')) {
                        return true;
                }

                start += target_len;
        }
        return false;
}

#define ALIASES_LEN 12

const_str *const ALIASES[ALIASES_LEN] = {
    (const_str[2]){"c", "clean"},

    (const_str[2]){"d", "dev"},
    (const_str[2]){"g", "debug"},
    (const_str[2]){"r", "release"},
    (const_str[2]){"u", "run"},

    (const_str[2]){"ii", "init"},
    (const_str[2]){"i", "install"},

    (const_str[2]){"a", "start"},
    (const_str[2]){"o", "stop"},
    (const_str[2]){"e", "enable"},
    (const_str[2]){"s", "disable"},

    (const_str[2]){"v", "valgrind"},
};

static void try_make(const_str target, const_str makefile) {
        const char *expanded_target = target;

        for (size_t i = 0; i < ALIASES_LEN; ++i) {
                const_str *const alias = ALIASES[i];

                if (!strcmp(alias[0], expanded_target)) {
                        expanded_target = alias[1];
                        break;
                }

                if (!strcmp(alias[1], expanded_target))
                        upanic("use m %s", alias[0]);
        }

        FILE *fd = fopen(makefile, "r");
        if (fd == NULL) return;

        const size_t target_len = strlen(expanded_target);
        char line[128];

        while (fgets(line, 128, fd)) {
                if (line_has_target(line, expanded_target, target_len)) {
                        fclose(fd);
                        exldn("make", "-j20", "-f", makefile, expanded_target);
                }
        }

        fclose(fd);
}

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

        snprintf(path, 32, "/usr/share/man/%s", folder_name);

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

        if (!c_man && strcmp(argv[0], "m"))
                upanic("Erroneous program name. Should be "
                       "either m or "
                       "mc, but "
                       "%s was found.",
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
                printf("Valid aliases:\n");
                for (int i = 0; i < ALIASES_LEN; ++i)
                        printf("  %s\t-> %s\n", ALIASES[i][0], ALIASES[i][1]);
                exit(1);
        }
}

static int mount(const int argc, Args argv) {
        if (argc > 4 || argc < 3)
                upanic("Usage: %s [--mkdir] %s "
                       "<destination>",
                       argv[0],
                       argv[1]);

        exldn("sudo", "mount", argv[1], argv[2], argv[3]);
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

        if (normalised_argc == 1) exldn("make", "-j");

        if (is_mount) return mount(normalised_argc, argv);

        if (normalised_argc == 3)
                try_make(argv[1], argv[2]);
        else
                try_make(argv[1], "Makefile");

        if (clear) system("clear");

        const size_t nb = get_man_pages(c_man, argv[1]);
        if (nb == 0) exldn("make", "-j", argv[1]);

        return 0;
}
