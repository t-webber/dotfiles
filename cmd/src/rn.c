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
        sprintf(path,
                "%s/%ld%ld/",
                waste,
                ts.tv_sec,
                ts.tv_nsec);
        printf("%s => %s\n", filename, path);
        mkdir(path, 0755);

        exl("/bin/mv", "mv", filename, path);
}

int main(int argc, const_str *const argv) {
        store_usage(argv);
        if (argc == 1) panic("Missing arguments...\n");

        pid_t *pids
            = malloc(sizeof(pid_t) * (size_t)(argc - 1));

        for (int i = 0; i < argc - 1;) {
                pid_t pid = fork_checked();
                pids[i] = pid;
                ++i;
                if (pid == 0) { rm_file(argv[i]); }
        }

        for (int i = 0; i < argc - 1; ++i) {
                fork_wait(pids[i]);
        }

        return 0;
}
