#include "lib.h"
#include "libexec.h"
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static void rm_file(const_str filename) {
        const_str waste = getenv_checked("WASTE");
        mkdir(waste, 0755);

        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        char path[256];
        sprintf(path, "%s/%ld%ld/", waste, ts.tv_sec, ts.tv_nsec);
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
                panic("invalid unit, use h or d")
        }
}

static void clean_waste(const unsigned long long amount) {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        char time[32];
        sprintf(time, "%ld%ld/", ts.tv_sec, ts.tv_nsec);
        const unsigned long long since_unix = strtoull(time, NULL, 10);
        const unsigned long long boundary = since_unix - amount;
        printf("Deleting under %lld\n", boundary);
}

int main(int argc, const_str *const argv) {
        store_usage(argv);
        const bool clear = is_verbose(argv[0], "rn", "rnc");
        if (argc == 1) panic("Missing arguments...");

        if (clear) {
                if (argc != 3) panic("Usage: mc <h|j|s|m> <amout>");
                clean_waste(unit_amount(*argv[1])
                            * strtoull(argv[2], NULL, 10));
                return 0;
        }

        pid_t *pids = malloc(sizeof(pid_t) * (size_t)(argc - 1));

        for (int i = 0; i < argc - 1;) {
                pid_t pid = fork_checked();
                pids[i] = pid;
                ++i;
                if (pid == 0) { rm_file(argv[i]); }
        }

        for (int i = 0; i < argc - 1; ++i) { fork_wait(pids[i]); }

        return 0;
}
