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

#define ALIASES_LEN 5
const char *const *const ALIASES[ALIASES_LEN] = {
    (const char *const[2]){"c", "clean"},
    (const char *const[2]){"rl", "release"},
    (const char *const[2]){"d", "debug"},
    (const char *const[2]){"g", "debug"},
    (const char *const[2]){"r", "run"},
};

static void try_make(const char *const target, const char *const makefile) {
        const char *expanded_target = target;

        for (size_t i = 0; i < ALIASES_LEN; ++i)
                if (!strcmp(ALIASES[i][0], expanded_target)) {
                        expanded_target = ALIASES[i][1];
                        break;
                }

        printf("target = %s\n", expanded_target);

        FILE *fd = fopen(makefile, "r");
        if (fd == NULL)
                return;

        const size_t target_len = strlen(expanded_target);
        char line[128];

        while (fgets(line, 128, fd))
                if (line_has_target(line, expanded_target, target_len)) {
                        fclose(fd);
                        int res = execl("/usr/bin/make", "make", "-f", makefile,
                                        expanded_target, NULL);
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

static void parse_argv(int *const argc, const char *const *const argv,
                       bool *const c_man) {
        *c_man = !strcmp(argv[0], "mc");

        if (!c_man && strcmp(argv[0], "m"))
                panic("Erroneous program name. Should be either m or "
                      "mc, but "
                      "%s was found.\n",
                      argv[0]);

        bool no_clear = !strcmp(argv[*argc - 1], "!");

        if (no_clear)
                --*argc;

        bool invalid_args = (*argc == 2 && !strcmp(argv[1], "?")) ||
                            (*argc > 3) || (*c_man && *argc != 2);
        if (invalid_args) {
                printf("Usage: %s (? | [(<target>|<alias>) [<Makefile>]] | "
                       "<page-name> | "
                       "(<device> <path>)) [!]\n\nValid aliases:\n",
                       argv[0]);
                for (int i = 0; i < ALIASES_LEN; ++i)
                        printf("  %s\t-> %s\n", ALIASES[i][0], ALIASES[i][1]);
                exit(1);
        }

        if (!no_clear)
                system("clear");
}

static int mount(const int argc, const char *const *const argv) {

        if (argc != 3)
                panic("Usage: %s %s <destination>\n", argv[0], argv[1]);

        return execl("/usr/bin/sudo", "sudo", "mount", argv[1], argv[2], NULL);
}

int main(const int argc, const char *const *const argv) {
        store_usage(argv);
        bool c_man;
        int normalised_argc = argc;
        parse_argv(&normalised_argc, argv, &c_man);

        if (normalised_argc == 1)
                return make();

        if (argv[1][0] == '/')
                return mount(normalised_argc, argv);

        if (normalised_argc == 3)
                try_make(argv[1], argv[2]);
        else
                try_make(argv[1], "Makefile");

        const size_t nb = get_man_pages(c_man, argv[1]);
        if (nb == 0)
                panic("No man page found with name %s.\n", argv[1]);
        return 0;
}
