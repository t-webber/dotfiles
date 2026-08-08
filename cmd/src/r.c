#include "lib.h"
#include "libexec.h"

int main(int argc, Args argv) {
        store_usage(argv[0], "", false);
        if (argc == 1) upanic("Missing arguments...");
        bool all_ok = true;

        pid_t *pids = malloc(sizeof(pid_t) * (size_t)(argc - 1));

        for (size_t i = 0; i < (size_t)argc - 1; ++i) {
                const_str path = argv[i + 1];
                pid_t pid = fork_checked();
                if (pid == 0) { rn_file(path); }
                pids[i] = pid;
        }

        for (size_t i = 0; i < (size_t)argc - 1; ++i) {
                if (pids[i]) fork_wait(pids[i]);
        }

        return all_ok ? 0 : 1;
}
