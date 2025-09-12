#include "lib.h"
#include "libexec.h"
#include <dirent.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static uint64_t since_unix(void) {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        const uint64_t nsec = (uint64_t)ts.tv_nsec;
        const uint64_t sec = (uint64_t)ts.tv_sec;
        return (nsec + sec * 1000 * 1000 * 1000);
}

static void rn_file(const_str filename) {
        const_str waste = getenv_checked("WASTE");
        mkdir(waste, 0755);

        char path[256];
        snprintf(path, sizeof path, "%s/%" PRIu64 "/", waste, since_unix());
        printf("%s => %s\n", filename, path);
        mkdir(path, 0755);

        exldn("mv", filename, path);
}

static uint64_t unit_amount(const char unit) {
        const uint64_t sec = 1e9;

        switch (unit) {

        case 'h':
                return 3600 * sec;

        case 'd':
                return 24 * 3600 * sec;

        default:
                upanic("invalid unit, use h or d")
        }
}

static void clean_waste(const uint64_t amount) {
        const uint64_t boundary = since_unix() - amount;
        printf("Deleting under %" PRIu64, boundary);

        DIR *bin = opendir_checked(getenv_checked("WASTE"));
        const struct dirent *file;

        while ((file = readdir(bin))) {
                if (file->d_name[0] == '.') continue;
                const size_t date = (size_t)atoi(file->d_name);
                if (date < boundary) printf("removing %s\n", file->d_name);
        }
}

int main(int argc, Args argv) {
        store_usage(argv);
        const bool clear = is_verbose(argv[0], "rn", "rnc");

        if (clear) {
                if (argc == 2 && !strcmp(argv[1], "all")) {
                        chdir(getenv_checked("WASTE"));
                        system("fd --hidden --ignore | while read p; sleep "
                               "0.01; sudo rm -rf $p & done");
                        exit(0);
                }
                if (argc != 3) upanic("Usage: rnc <all|<h|j|s|m> <amout>>\n");
                clean_waste(unit_amount(*argv[1])
                            * strtoull(argv[2], NULL, 10));
                return 0;
        }

        if (argc == 1) upanic("Missing arguments...");

        pid_t *pids = malloc(sizeof(pid_t) * (size_t)(argc - 1));

        for (int i = 0; i < argc - 1;) {
                pid_t pid = fork_checked();
                pids[i] = pid;
                ++i;
                if (pid == 0) { rn_file(argv[i]); }
        }

        for (int i = 0; i < argc - 1; ++i) { fork_wait(pids[i]); }

        return 0;
}
