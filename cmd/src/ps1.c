#include "lib.h"
#include "libexec.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define SHORT(path, alias, condition)                                          \
        if (condition) {                                                       \
                stpcpy(path, alias);                                           \
                return;                                                        \
        }

#define SHORTP(path, pwd, var, alias)                                          \
        SHORT(path, alias, strcmp(getenv_checked(var), pwd) == 0);

#define SHORTE(path, pwd, target, alias)                                       \
        SHORT(path, alias, strcmp(target, pwd) == 0);

static char *exec(const_str command) {
        char *output = malloc(200 * sizeof(char));
        *output = '\0';
        FILE *f = popen(command, "r");
        fgets(output, 199, f);
        pclose(f);
        return output;
}

static void pwd(str path) {
        read_simple_exl1(512, pwd, "pwd");
        if (pwd == NULL || strlen(pwd) == 0) {
                *path = '\0';
                return;
        }
        assert(strlen(pwd) >= 1);
        const size_t len = strlen(pwd);
        char *end = pwd + len;
        for (; *end != '\n'; --end);
        *end = '\0';

        SHORTP(path, pwd, "HOME", "~");
        SHORTP(path, pwd, "FILES", "f");
        SHORTP(path, pwd, "CMD", "c");
        SHORTP(path, pwd, "APPS", "a");
        SHORTP(path, pwd, "DEV", "d");
        SHORTP(path, pwd, "DATA", "t");
        SHORTP(path, pwd, "DOT", ".");

        while (*end != '/' && *end != '.') --end;

        if (end != pwd || *(end + 1) != '\0') ++end;

        const char *dir = end;

        if (!strcmp(dir, "jet-centre")) dir = "jc";
        if (!strcmp(dir, "showcase-website")) dir = "sw";
        if (!strcmp(dir, "master")) dir = ":";
        if (!strcmp(dir, "main")) dir = ".";
        if (!strcmp(dir, "delme")) dir = "!";

        stpcpy(path, dir);

        free(pwd);
}

static char *get_git_branch(void) {
        char *branch = exec("/usr/bin/git branch "
                            "--show-current 2>/dev/null");
        if (*branch == '\0') return branch;

        *(branch + strlen(branch) - 1) = '\0';

        if (strcmp(branch, "main") == 0) stpcpy(branch, ".");
        if (strcmp(branch, "master") == 0) stpcpy(branch, ":");
        if (strcmp(branch, "delme") == 0) stpcpy(branch, "!");

        return branch;
}

static void low_battery(void) {
        pid_t pid_outer = fork_checked();

        if (pid_outer == 0) {
                pid_t pid_inner = fork_checked();

                if (pid_inner == 0) { exl_err_notif_msg("Low battery (ps1)"); }

                fork_wait(pid_inner);

                const_str status = get_battery_status();
                const int level = atoi(get_battery_level());

                if (!strcmp(status, "Discharging") && level < 10) {
                        exldn("sudo", "systemctl", "suspend");
                }

                if (level < 10) {}
        }

        fork_wait(pid_outer);
}

int main(void) {
        const char *device_name = getenv_checked("DEVICE");
        const bool is_acer = !strcmp(device_name, "acer");

        char *const battery = get_battery_level();
        char *const status = get_battery_status();
        const bool is_charging = status && !strcmp(status, "Charging");
        if (status) free(status);

        if (battery && !strcmp(battery, "100"))
                exl_err_notif_msg("Battery full");

        if (!is_charging && is_acer && battery && atoi(battery) < 10)
                low_battery();

        if (!strcmp(device_name, "acer") || !strcmp(device_name, "mac"))
                device_name = "";

        time_t t = time(NULL);
        struct tm tm = *localtime(&t);

        char path[50];
        pwd(path);

        char *branch = get_git_branch();

        if (battery && battery[0] != '\0') battery[strlen(battery) - 1] = '\0';

        printf("\001\x1b[35m\002%s"
               "\001\x1b[%sm%s\002"
               "\001\x1b[33m\002%x%d"
               "\001\x1b[36m\002%s"
               "\001\x1b[32m\002%s\001\x1b[39m\002",
               device_name,
               is_charging ? "32" : "31",
               battery == NULL ? "" : battery + 0,
               tm.tm_hour % 12,
               tm.tm_min,
               path,
               branch);

        free(branch);

        return 0;
}
