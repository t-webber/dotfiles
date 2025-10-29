#include "lib.h"
#include "libexec.h"
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static unsigned long long since_unix(void) {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        const unsigned long long nsec = (unsigned long long)ts.tv_nsec;
        const unsigned long long sec = (unsigned long long)ts.tv_sec;
        return (nsec + sec * 1000 * 1000 * 1000);
}

static void rn_file(const_str filename) {
        const_str waste = getenv_checked("WASTE");
        mkdir(waste, 0755);

        char path[256];
        sprintf(path, "%s/%lld/", waste, since_unix());
        printf("%s => %s\n", filename, path);
        mkdir(path, 0755);

        exl("/bin/mv", "mv", filename, path);
}

static unsigned long long unit_amount(const char unit) {
        const unsigned long long sec = 1e9;

        switch (unit) {

        case 'h':
                return 3600 * sec;

        case 'd':
                return 24 * 3600 * sec;

        default:
                upanic("invalid unit, use h or d")
        }
}

static void clean_waste(const unsigned long long amount) {
        const unsigned long long boundary = since_unix() - amount;
        printf("Deleting under %lld\n", boundary);

        DIR *bin = opendir_checked(getenv_checked("WASTE"));
        const struct dirent *file;

        while ((file = readdir(bin))) {
                if (file->d_name[0] == '.') continue;
                const size_t date = (size_t)atoi(file->d_name);
                if (date < boundary) printf("removing %s\n", file->d_name);
        }
}

int main(int argc, const_str *const argv) {
        store_usage(argv);
        const bool clear = is_verbose(argv[0], "rn", "rnc");

        if (clear) {
                if (argc != 3) upanic("Usage: mc <h|j|s|m> <amout>");
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
