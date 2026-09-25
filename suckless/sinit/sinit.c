#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define folder "/del/.dot/suckless/sinit/"
#define LEN(x) (sizeof(x) / sizeof *(x))
#define TIMEO 30

static sigset_t set;

static void spawn(char *const argv[]) {
        switch (fork()) {
        case 0:
                sigprocmask(SIG_UNBLOCK, &set, NULL);
                setsid();
                execvp(argv[0], argv);
                perror("execvp");
                _exit(1);
        case -1:
                perror("fork");
        }
}

static void sigpoweroff(void) {
        spawn((char *[]){folder "shutdown", "poweroff", NULL});
}

static void sigreap(void) {
        while (waitpid(-1, NULL, WNOHANG) > 0);
        alarm(TIMEO);
}

static void sigreboot(void) {
        spawn((char *[]){folder "shutdown", "reboot", NULL});
}

static struct {
        int sig;
        void (*handler)(void);
} sigmap[] = {
    {SIGUSR1, sigpoweroff},
    {SIGCHLD, sigreap},
    {SIGALRM, sigreap},
    {SIGINT, sigreboot},
};

int main(void) {
        int sig;
        chdir("/");
        sigfillset(&set);
        sigprocmask(SIG_BLOCK, &set, NULL);
        spawn((char *[]){folder "setup.sh", NULL});
        while (1) {
                alarm(TIMEO);
                sigwait(&set, &sig);
                for (size_t i = 0; i < LEN(sigmap); ++i)
                        if (sigmap[i].sig == sig) {
                                sigmap[i].handler();
                                break;
                        }
        }
        return 0;
}
