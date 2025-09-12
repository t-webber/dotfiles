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

static bool can_use_dunst(void) {
        return getenv("NO_DUNST") != NULL;
}

#define ghprefix(str, short)                                                   \
        {                                                                      \
                size_t prefix = sizeof(str) - 1;                               \
                if (!strncmp(branch, str, prefix)) {                           \
                        char *ptr = stpcpy(branch, short);                     \
                        char *ch = branch + prefix;                            \
                        for (; *ch >= '0' && *ch <= '9'; ++ch) {               \
                                *ptr++ = *ch;                                  \
                        }                                                      \
                        *ptr = '\0';                                           \
                }                                                              \
        }

static char *get_git_branch(void) {
        char *branch = exec("/usr/bin/git branch "
                            "--show-current 2>/dev/null");
        if (*branch == '\0') return branch;

        *(branch + strlen(branch) - 1) = '\0';

        if (strcmp(branch, "main") == 0) stpcpy(branch, ".");
        if (strcmp(branch, "master") == 0) stpcpy(branch, ":");
        if (strcmp(branch, "delme") == 0) stpcpy(branch, "!");

        ghprefix("user/twebber/SW-", "sw");
        ghprefix("user/twebber/AUTODIAG-", "ad");
        ghprefix("user/twebber/", "w-");

        return branch;
}

static void low_battery(void) {
        pid_t pid_outer = fork_checked();

        if (pid_outer != 0) fork_wait(pid_outer);

        if (can_use_dunst()) {
                pid_t pid_inner = fork_checked();

                if (pid_inner == 0) { exl_err_notif_msg("Low battery (ps1)"); }

                fork_wait(pid_inner);
        }

        const battery_status status = get_battery_status();
        const int level = atoi(get_battery_level());

        if (status == BATTERY_STATUS_DISCHARGING && level < 10) {
                exldn("sudo", "systemctl", "suspend");
        }
}

#define COL(x) "\001\x1b[" #x "m\002"

int main(void) {
        const char *device_name = getenv_checked("DEVICE");
        const bool is_acer = !strcmp(device_name, "acer");

        char *const battery = get_battery_level();
        const battery_status status = get_battery_status();

        if (battery && !strcmp(battery, "100") && can_use_dunst())
                exl_err_notif_msg("Battery full");

        if (status == BATTERY_STATUS_DISCHARGING && is_acer && battery
            && atoi(battery) < 10)
                low_battery();

        const_str battery_colour = status == BATTERY_STATUS_DISCHARGING
                                       ? COL(31)
                                   : status == BATTERY_STATUS_CHARGING ? COL(32)
                                   : status == BATTERY_STATUS_FULL     ? COL(35)
                                                                       : "";

        if (!strcmp(device_name, "acer") || !strcmp(device_name, "mac"))
                device_name = "";

        time_t t = time(NULL);
        struct tm tm = *localtime(&t);

        char path[50];
        pwd(path);

        char *branch = get_git_branch();

        if (battery && battery[0] != '\0') battery[strlen(battery) - 1] = '\0';

        printf(COL(35) "%s%s%s" COL(33) "%x%d" COL(36) "%s" COL(32) "%s" COL(0),
               device_name,
               battery_colour,
               battery == NULL ? "" : battery + 0,
               (unsigned)tm.tm_hour % 12,
               tm.tm_min,
               path,
               branch);

        fflush(stdout);

        free(branch);
        return 0;
}
