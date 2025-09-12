#include "lib.h"
#include "libexec.h"
#include "libos.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

/// TODO: if folder is branch name, then display the parent with trailing <

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
        const_str pwd = getenv_checked("PWD");
        const size_t len = strlen(pwd);
        const char *end = pwd + len;

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

static void battery_warnings(const battery_status status, int battery) {
        const bool can_use_dunst = getenv("NO_DUNST") == NULL;

        if ((status == BATTERY_STATUS_FULL
             || (battery == 100 && status == BATTERY_STATUS_CHARGING))
            && can_use_dunst)
                if (!fork_and_wait()) exl_err_notif("Battery full");

        if (status == BATTERY_STATUS_DISCHARGING && battery < 20
            && can_use_dunst)
                if (!fork_and_wait()) exl_err_notif("Low battery (ps1)");

        if (status == BATTERY_STATUS_DISCHARGING && battery < 10) {
                if (is_file("/etc/artix-release")) {
                        if (!fork_and_wait()) exldn("loginctl", "suspend");
                } else {
                        if (!fork_and_wait())
                                exldn("sudo", "systemctl", "suspend");
                }
        }
}

// /r/ do not remove /r/
#define COL(x) "\001\x1b[" #x "m\002"
// /r/ do not remove /r/

__wur static char get_battery(const char **const colour) {
        const battery_status status = get_battery_status();
        const char *battery = get_battery_level();
        if (battery == NULL) battery = "??";

        *colour = (status == BATTERY_STATUS_FULL || !strcmp(battery, "100"))
                      ? COL(35)
                  : status == BATTERY_STATUS_DISCHARGING ? COL(31)
                  : status == BATTERY_STATUS_CHARGING    ? COL(32)
                                                         : COL(33);

        if (battery) battery_warnings(status, atoi(battery));
        return strlen(battery) != 2 ? '0' : battery[0];
}

__wur static const char *get_dev(void) {
        const char *dev = getenv_checked("DEVICE");
        if (!strcmp(dev, "acer") || !strcmp(dev, "mac")) return "";
        return dev;
}

int main(void) {
        store_usage("ps1", "", false);

        const_str dev = get_dev();

        const char *bat_col;
        const char bat = get_battery(&bat_col);

        time_t t = time(NULL);
        struct tm tm = *localtime(&t);

        char path[50];
        pwd(path);

        char *const branch = get_git_branch();

        const_str venv = getenv("VIRTUAL_ENV") ? "v" : "";
        const_str root = !strcmp(getenv_checked("USER"), "root") ? "#" : "";

        printf(COL(34) "%s%s" COL(35) "%s%s%c" COL(33) "%x%d" COL(36) "%s" COL(
                   32) "%s" COL(0),
               venv,
               root,
               dev,
               bat_col,
               bat,
               (unsigned)tm.tm_hour % 12,
               tm.tm_min,
               path,
               branch);

        fflush(stdout);
        free(branch);
        return 0;
}
