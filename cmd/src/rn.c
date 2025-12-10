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

const uint64_t NS = 1;
const uint64_t SEC = 1000 * 1000 * 1000 * NS;

__wur static uint64_t since_unix(void) {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        const uint64_t nsec = (uint64_t)ts.tv_nsec;
        const uint64_t sec = (uint64_t)ts.tv_sec;
        return (nsec * NS + sec * SEC);
}

_Noreturn __nonnull() static void rn_file(const_str filename) {
        const_str waste = getenv_checked("WASTE");
        mkdir(waste, 0755);

        char path[256];
        snprintf(path, sizeof(path), "%s/%" PRIu64 "/", waste, since_unix());
        printf("%s => %s\n", filename, path);
        mkdir(path, 0755);

        exldn("mv", filename, path);
}

int main(int argc, Args argv) {
        store_usage(argv);
        if (argc == 1) upanic("Missing arguments...");
        bool all_ok = false;

        pid_t *pids = malloc(sizeof(pid_t) * (size_t)(argc - 1));

        for (int i = 0; i < argc - 1;) {
                pid_t pid = fork_checked();
                pids[i] = pid;
                ++i;
                if (pid == 0) { rn_file(argv[i]); }
                all_ok = all_ok && (access(argv[i], F_OK));
        }

        for (int i = 0; i < argc - 1; ++i) { fork_wait(pids[i]); }

        return all_ok ? 0 : 1;
}
