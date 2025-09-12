#include "libexec.h"
#include <sys/wait.h>

pid_t fork_checked(void) {
        pid_t pid = fork();
        if (pid < 0) epanic("Failed to fork");

        return pid;
}

void fork_wait(pid_t pid) {
        int status;
        waitpid(pid, &status, 0);

        if (!WIFEXITED(status)) {
                upanic("Unexpected panic on pid %d and "
                       "status %d",
                       pid,
                       status);
        }
}

_Noreturn void exvd(Args args) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
        str *const non_const_args = (str *const)args;
#pragma GCC diagnostic pop
        int res = execvp(args[0], non_const_args);
        epanic("Failed to execute %s: exicted with code %d", args[0], res);
}

__nonnull() void forked_exvd(Args args) {
        pid_t pid = fork_checked();
        if (pid == 0) exvd(args);
        fork_wait(pid);
}
