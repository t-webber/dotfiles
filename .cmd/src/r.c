#include "lib.h"
#include <dirent.h>
#include <sys/wait.h>
#include <unistd.h>

#define exec(cmd, ...)                                                         \
        {                                                                      \
                execlp(cmd, __VA_ARGS__, NULL);                                \
                panic("Failed to execute %s.\n", cmd)                          \
        }

#define run_ext(ext, cmd)                                                      \
        if (!strcmp(extension, ext))                                           \
                exec(cmd, file);

static int run_file(const char *const file, const char *const extension) {

        if (!strcmp(extension, "c") || !strcmp(extension, "cc") ||
            !strcmp(extension, "cpp")) {
                pid_t pid = fork();

                if (pid == 0)
                        exec("cc", "cc", file, "-o", "a.out");

                int status;
                waitpid(pid, &status, 0);
                if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
                        exec("./a.out", "./a.out");

                panic("Unexpected panic on pid %d and status %d.\n", pid,
                      status);
        }

        run_ext("py", "python");
        run_ext("sage", "sage");
        run_ext("html", "brave");

        FILE *fd = fopen(file, "r");
        if (fd == NULL)
                panic("Failed to open %s.\n", file);

        char line[64];
        char *buf = fgets(line, 64, fd);
        fclose(fd);
        if (buf == NULL)
                panic("Failed to read first line of %s.\n", file);

        line[strcspn(line, "\n")] = '\0';
        exec(line + 2, line + 2, file);
}

static int run_folder(void) {

        char cwd[128];
        char *buf = getcwd(cwd, 128);
        if (buf == NULL)
                panic("Failed to get current working directory\n");

        DIR *dirp = opendir(cwd);
        struct dirent *entry;
        while ((entry = readdir(dirp))) {
        }
}

int main(const int argc, const char *const *const argv) {
        if (argc > 2)
                panic("Usage: %s [<filename>]\n", argv[0]);

        if (argc == 1)
                return run_folder();

        const char *const file = argv[1];
        const char *extension = file;

        for (const char *ptr = file; *ptr != '\0'; ++ptr)
                if (*ptr == '.')
                        extension = ptr + 1;

        return run_file(file, extension);
}
