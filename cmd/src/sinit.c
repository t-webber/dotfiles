#include "lib.h"
#include "libexec.h"
#include "libos.h"
#include "libvec.h"
#include <fcntl.h>
#include <grp.h>
#include <linux/limits.h>
#include <pwd.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

/// UTILS ///

#define user "b"
#define f "/del/"
#define t f ".data/"
#define d f ".dot/"
#define logs t ".logs"
#define folder d "/suckless/sinit/"
#define exe(fn) _Noreturn static void ex_##fn(void)

__nonnull() static void tprint(const_str msg, const uint64_t start) {
        const uint64_t end = since_unix();
        static bool first = true;
        printf("\x1b[33m");
        if (first) {
                printf("┌");
                first = false;
        } else {
                printf("├");
        }
        long double ms = (long double)(end - start) / (long double)1e6;
        printf("WebberOS:%s\x1b[0m (%.2Lfms)\n", msg, ms);
}

/// SETUP ///

exe(stage1) {
#define parent "/etc/rc/sysinit/"
        Vec files = alphascan_dir(parent);
        char path[PATH_MAX];
        char *const wrt = stpcpy(path, parent);
        for (size_t i = 0; i < files.len; ++i) {
                const_str name = files.data[i];
                if (strcmp(name, ".") && strcmp(name, "..")) {
                        stpcpy(wrt, name);
                        forked_exldn(path, "start");
                };
                free(unsafe_const_cast(files.data[i]));
        }
        exit(0);
#undef parent
}

static void vars(void) {
#define sv(var, val) \
        if (setenv(#var, val, 1)) perror("sinit: var: " #var)

        sv(SHELL, "/bin/bash");
        sv(EDITOR, "/bin/nvim");
        sv(DEVICE, "acer");

        sv(LOGS, logs);
        sv(FILES, f);
        sv(WASTE, f ".waste");
        sv(APPS, f ".apps");
        sv(DEV, f ".dev");
        sv(SECRET, f ".secret");
        sv(STUDY, f ".study");
        sv(BIN, f ".bin");
        sv(BLOB, f ".blob");
        sv(WORK, f ".work");

        sv(DOT, d);
        sv(CMD, d "cmd");
        sv(CMD_SRC, d "cmd/src");
        sv(XDG_CONFIG_HOME, d "xdgconfig");
        sv(ETC, d "etc");
        sv(OCFG, d "otherconfig");

        sv(DATA, t);
        sv(XDG_DATA_HOME, f ".share");
        sv(XDG_CACHE_HOME, f ".cache");
        sv(XDG_STATE_HOME, f ".state");
        sv(XDG_RUNTIME_DIR, f "rt");
        sv(RUSTUP_HOME, f "rustup");
        sv(CARGO_HOME, f "cargo");
        sv(BUN_INSTALL, f "bun");

#define p(dir)                                     \
        if (is_dir(dir)) {                         \
                extend_s(&path, dir, sizeof(dir)); \
                push_s(&path, ':');                \
        }

#define pp(var, left)                                             \
        {                                                         \
                var_prefix(dir, getenv_checked(#var), "/" #left); \
                p(dir);                                           \
        }

        String path = new_s();
        pp(CMD, bin);
        p("bin");
        p(getenv_checked("BIN"));
        pp(CMD, old);
        pp(CMD, sh);
        pp(CARGO_HOME, bin);
        pp(BUN_INSTALL, bin);
#undef sv
#undef p
#undef pp
}

exe(setup) {
        uint64_t start = since_unix();
        safe_ex1(true, "setfont");
        tprint("setfont", start);
        int file = open(folder "issue", O_WRONLY);
        if (file < 0) perror("sinit: open issue");
        sendfile(STDOUT_FILENO, file, 0, 1 << 30);
        printf("\x1b[0m");
        if (close(file)) perror("sinit: close issue");
        tprint("setup", start);
        start = since_unix();

        vars();
        tprint("vars", start);
        start = since_unix();

        safe_fork(true, "logs", {
                if (!fork_and_wait()) rn_file(logs ".old");
                rename(logs, logs ".old");
                mkdir(logs, 0777);
                chmod(logs, 0777);
                exit(0);
        });
        tprint("logs", start);
        start = since_unix();

        safe_fork(true, "stage1", ex_stage1());
        tprint("stage1", start);
        start = since_unix();

        safe_exl(false, "agetty", "--noclear", "tty2", "-f", folder "issue", "linux");
        safe_exl(false, "agetty", "--noclear", "tty3", "-f", folder "issue", "linux");
        tprint("agetty", start);
        start = since_unix();

        struct passwd *p = getpwnam(user);
        if (!p || initgroups(p->pw_name, p->pw_gid) || setgid(p->pw_gid) || setuid(p->pw_uid))
                epanic("failed to de-escalated from root to user " user);
        tprint("user", start);
        start = since_unix();

        uid_t r, e, s;
        gid_t rg, eg, sg;
        getresuid(&r, &e, &s);
        getresgid(&rg, &eg, &sg);
        printf("\x1b[32m uid %u/%u/%u gid %u/%u/%u\n", r, e, s, rg, eg, sg);
        setsid();                           // new session, no ctty
        int ff = open("/dev/tty1", O_RDWR); // pick a VT with no getty on it
        if (ff < 0 || ioctl(ff, TIOCSCTTY, 0) < 0) epanic("tty1");
        dup2(ff, 0);
        dup2(ff, 1);
        dup2(ff, 2);
        if (ff > 2) close(ff);
        safe_exl(false,
                 "xinit",
                 folder "setup.xinit.sh",
                 "--",
                 "/usr/lib/Xorg",
                 "vt1",
                 "-keeptty",
                 "-logfile",
                 logs "/x-xorg");
        tprint("xorg", start);
        start = since_unix();

        safe_fork(true, "net", {
                int fd = open_checked(logs "/l-net", O_WRONLY | O_TRUNC | O_CREAT);
                dup2(fd, STDOUT_FILENO);
                dup2(fd, STDERR_FILENO);
                close_checked(fd);
                exldn("net", "au");
        });
        tprint("net", start);
        start = since_unix();

        FILE *fd = fopen_checked(logs "/l-lwd", "w");
        fputs("/", fd);
        fclose_checked(fd);

        fd = fopen_checked(logs "/i-date", "w");
        time_t ti = time(0);
        fputs(ctime(&ti), fd);
        fclose_checked(fd);
        tprint("files", start);

        exit(0);
}

/// SIGNAL LOOP ///

exe(signal_loop) {
        sigset_t set;
        int sig;
        sigfillset(&set);                   // select all signals
        sigprocmask(SIG_BLOCK, &set, NULL); // block all signals (needed for sigwait).
        while (1) {
                alarm(30);
                sigwait(&set, &sig);
                switch (sig) {
                case SIGALRM: /* from alarm */
                case SIGCHLD: /* from kernel */
                        while (waitpid(-1, NULL, WNOHANG) > 0);
                        continue;
                case SIGUSR1:
                case SIGINT: {
                        const_str action = sig == SIGINT ? "reboot" : "poweroff";
                        pid_t pid = fork();
                        if (pid == -1) perror("sinit: failed to fork for shutdown");
                        if (pid != 0) continue;
                        sigprocmask(SIG_UNBLOCK, &set,
                                    NULL); // liberate signals for new process
                        setsid();          // separate the process terminal
                        exldn("shutdown", folder "shutdown", action, NULL);
                }
                default:
                        continue;
                }
        }
}

/// MAIN ///

int main(void) {
        if (chdir("/")) perror("sinit: failed to chdir to /"); // cwd of process blocks umount
        safe_fork(false, "setup", ex_setup());
        ex_signal_loop();
}
