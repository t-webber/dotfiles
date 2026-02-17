#include "libexec.h"
#include "lib.h"
#include "libos.h"

#include <sys/wait.h>

__wur bool is_dbg(void) {
        return getenv("DEBUG") != NULL;
}

__wur pid_t fork_checked(void) {
        pid_t pid = fork();
        if (pid < 0) epanic("Failed to fork");

        return pid;
}

__wur bool fork_and_wait(void) {
        pid_t pid = fork_checked();
        if (pid != 0) fork_wait(pid);
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
        if (is_dbg()) { print_inline_array(args); }
        int res = execvp(args[0], non_const_args);
        epanic("Failed to execute %s: exicted with code %d", args[0], res);
}

__nonnull() void forked_exvd(Args args) {
        if (fork_and_wait()) exvd(args);
}

_Noreturn __nonnull((1)) static void macos_notif(const_str message,
                                                 const char *title) {
        if (title == NULL) title = "C error";
        char osascript[512];
        sprintf(osascript,
                "display notification \"%s\" with title \"%s\"",
                message,
                title);
        exldn("osascript", "-e", osascript);
}

__nonnull() _Noreturn void exl_notif(const_str message) {
        if (is_file("/bin/notify-send"))
                exldn("notify-send",
                      "-u",
                      "low",
                      "-t",
                      "500",
                      "-h",
                      "string:x-dunst-stack-tag:stacked",
                      "-a",
                      "center",
                      message);
        macos_notif(message, NULL);
}

_Noreturn __nonnull() void exl_err_notif(const_str err_msg) {
        if (is_file("/bin/notify-send"))
                exldn("notify-send", "-u", "critical", err_msg);
        macos_notif(err_msg, NULL);
}
