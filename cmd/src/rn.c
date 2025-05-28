#include "lib.h"
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static void rm_file(const char *const filename) {
        const char *const waste = getenv("WASTE");
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        char path[256];
        sprintf(path, "%s/%ld%ld/", waste, ts.tv_sec, ts.tv_nsec);
        printf("%s => %s\n", filename, path);
        mkdir(path, 0755);
        pid_t pid = fork();
        if (pid < 0)
                panic("Failed to fork\n");
        if (pid == 0)
                execl("/bin/mv", "mv", filename, path, NULL);
        int status;
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status))
                panic("Failed to move.\n");
}

int main(int argc, const char *const *const argv) {
        store_usage(argv);
        if (argc == 1)
                panic("Missing arguments...\n");

        for (int i = 1; i < argc; ++i)
                rm_file(argv[i]);

        return 0;
}
