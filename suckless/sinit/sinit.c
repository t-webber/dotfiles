#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define folder "/del/.dot/suckless/sinit/"

static sigset_t set;

static void spawn(char *const argv[]) {
        switch (fork()) {
        case 0:
                sigprocmask(SIG_UNBLOCK, &set, NULL); // liberate signals for new process
                setsid();                             // separate the process terminal
                execvp(argv[0], argv);
                perror("sinit execvp");
                _exit(1);
        case -1:
                perror("sinit fork");
        }
}

int main(void) {
        int sig;
        chdir("/");                         // cwd of process blocks umount
        sigfillset(&set);                   // select all signals
        sigprocmask(SIG_BLOCK, &set, NULL); // block all signals (needed for sigwait).
                                            // SIGKILL and SIGSTOP can't be stopped.
        spawn((char *[]){folder "setup.sh", NULL});
        while (1) {
                alarm(30);
                sigwait(&set, &sig);
                switch (sig) {
                case SIGALRM: /* from alarm */
                case SIGCHLD: /* from kernel */
                        while (waitpid(-1, NULL, WNOHANG) > 0);
                        continue;
                case SIGUSR1:
                        spawn((char *[]){folder "shutdown", "poweroff", NULL});
                        continue;
                case SIGINT:
                        spawn((char *[]){folder "shutdown", "reboot", NULL});
                }
        }
        return 0;
}
