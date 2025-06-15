#include "lib.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <zlib.h>

static bool line_has_target(const char *const line, const char *const target,
                            const size_t target_len) {

        if (line[0] == '\0' || line[0] == '\t' || line[0] == ' ' ||
            !strchr(line, ':'))
                return false;

        const char *start = line;
        const size_t line_len = strlen(line);

        while (start + target_len < line + line_len) {
                if (*start == ' ') {
                        ++start;
                        continue;
                };

                if (!strncmp(target, start, target_len) &&
                    (start[target_len] == ':' || start[target_len] == ' '))

                        return true;

                start += target_len;
        }
        return false;
}

static void try_make(const char *const target, const char *const makefile) {
        FILE *fd = fopen(makefile, "r");
        if (fd == NULL)
                return;

        const size_t target_len = strlen(target);
        char line[128];

        while (fgets(line, 128, fd))
                if (line_has_target(line, target, target_len)) {
                        fclose(fd);
                        int res = execl("/usr/bin/make", "make", "-f", makefile,
                                        target, NULL);
                        panic("Make panicked with code %d.\n", res);
                }

        fclose(fd);
}

static int make(void) { return execl("/usr/bin/make", "make", NULL); }

static bool open_man_page(const char *const full_path, bool only_c_pages) {
        gzFile fd = gzopen(full_path, "r");
        char line[128];

        gzgets(fd, line, 128);
        if (!strncmp(".so ", line, 4)) {
                line[strlen(line) - 1] = '\0';
                gzclose(fd);
                pid_t pid = fork();
                if (pid < 0)
                        panic("Failed to fork...\n");
                if (pid == 0)
                        return execlp("m", "m", line + 4, NULL);

                int status;
                waitpid(pid, &status, 0);
                if (!WIFEXITED(status))
                        panic("Failed to execute m %s.\n", line + 4);
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

        if (display) {

                pid_t pid = fork();
                if (pid < 0)
                        panic("Failed to fork.\n");
                if (pid == 0)
                        return execl("/usr/bin/man", "man", full_path, NULL);

                int status;
                waitpid(pid, &status, 0);
                if (!WIFEXITED(status))
                        panic("Failed to execute man on path %s.\n", full_path);
        }

        return display;
}

static size_t get_man_pages_in_folder(bool only_c_pages,
                                      const char *const page_name,
                                      const size_t page_name_len,
                                      const char *const folder_name) {
        const size_t folder_name_len = strlen(folder_name);
        size_t len = 0;

        char path[32];

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-overflow"
        sprintf(path, "/usr/share/man/%s", folder_name);
#pragma GCC diagnostic pop

        DIR *d = opendir(path);
        if (d == NULL)
                panic("Failed to open %s.\n", path);

        struct dirent *file;
        while ((file = readdir(d))) {
                if (file->d_type != DT_REG && file->d_type != DT_LNK)
                        continue;

                char *pos = strstr(file->d_name, page_name);
                if (pos == NULL)
                        continue;

                const size_t file_name_len = strlen(file->d_name);
                const char *const end = pos + page_name_len;

                if (pos != file->d_name || *end != '.')
                        continue;

                char *const full_path = malloc(
                    sizeof(char) * (sizeof("/usr/share/man/") +
                                    folder_name_len + 1 + file_name_len + 1));
                char *writer = stpcpy(full_path, "/usr/share/man/");
                writer = stpcpy(writer, folder_name);
                *writer++ = '/';
                writer = stpcpy(writer, file->d_name);

                if (open_man_page(full_path, only_c_pages))
                        ++len;
        }

        closedir(d);
        return len;
}

static size_t get_man_pages(bool only_c_pages, const char *const page_name) {
        const size_t page_name_len = strlen(page_name);

        DIR *d = opendir("/usr/share/man");
        if (d == NULL)
                panic("Failed to open /usr/share/man.\n");

        struct dirent *file;
        size_t len = 0;

        while ((file = readdir(d))) {
                if (!strcmp(file->d_name, ".") || !strcmp(file->d_name, "..") ||
                    file->d_type != DT_DIR)
                        continue;

                len += get_man_pages_in_folder(only_c_pages, page_name,
                                               page_name_len, file->d_name);
        }

        closedir(d);
        return len;
}

static void parse_argv(const int argc, const char *const *const argv,
                       bool *const c_man, bool *const file_make) {
        *c_man = !strcmp(argv[0], "mc");
        *file_make = !strcmp(argv[0], "mf");

        if (!c_man && strcmp(argv[0], "m"))
                panic("Erroneous program name. Should be either m or "
                      "mc, but "
                      "%s was found.\n",
                      argv[0]);

        if (*file_make && argc != 3)
                panic("Usage: %s <Makefile> [<make-target>]\n", argv[0]);
        if (!*file_make && argc > 2)
                panic("Usage: %s [<make-target|man-page>]\n", argv[0]);
}

int main(const int argc, const char *const *const argv) {
        printf("\033[2J\033[H");
        fflush(stdout);

        bool c_man, file_make;
        parse_argv(argc, argv, &c_man, &file_make);

        if (argc == 1)
                return make();

        if (file_make)
                try_make(argv[2], argv[1]);
        else
                try_make(argv[1], "Makefile");

        const size_t nb = get_man_pages(c_man, argv[1]);
        if (nb == 0)
                panic("No man page found with name %s.\n", argv[1]);
        return 0;
}
