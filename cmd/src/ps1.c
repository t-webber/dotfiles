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
        char *pwd = exec("pwd");
        if (pwd == NULL || strlen(pwd) == 0) {
                *path = '\0';
                return;
        }
        assert(strlen(pwd) >= 1);
        const size_t len = strlen(pwd);
        char *end = pwd + len;
        assert(*end == '\0');
        --end;
        assert(*end == '\n');
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

        stpcpy(path, dir);

        free(pwd);
}

static bool is_battery_charging(void) {
        char *status = get_battery_status();
        const bool is_charging = !strcmp(status, "Charging");
        free(status);
        return is_charging;
}

static void wait_and_suspend(void) {
        forked_exl("/bin/notify-send",
                   "notify-send",
                   "-u",
                   "critical",
                   "🪫 Battery low",
                   get_battery_level());

        printf("\033[31m"
               "Battery running out. Only %s%% left. Plug your computer."
               "\033[0m\n",
               get_battery_level());

        sleep(60);

        if (is_battery_charging() && atoi(get_battery_level()) >= 6) return;

        //         exl("/bin/sudo", "sudo", "systemctl", "suspend");
}

static void get_battery_string(str battery, const_str device) {

        if (!strcmp(device, "mac")) {
                //                 FILE *f = popen(
                //                     "pmset - g batt | w
                //                     'NR == 2 {print $3}'
                //                     | sed 's/%;//'",
                //                     "r");
                //
                //                 return;
        }

        if (!strcmp(device, "acer")) {
                str level = get_battery_level();
                const int level_value = atoi(level);
                if (level_value >= 79) return;

                const bool is_charging = is_battery_charging();

                char *end;
                if (is_charging) {
                        end = stpcpy(battery, "\001\x1b[32m\002");
                        if (level_value < 5) wait_and_suspend();
                } else {
                        end = stpcpy(battery, "\001\x1b[31m\002");
                        if (level_value < 10) wait_and_suspend();
                }

                *end++ = level[0];
                *end = '\0';
                return;
        }

        char *end = stpcpy(battery, "\001\x1b[31m\002");
        stpcpy(end, "");
}

static char *get_git_branch(void) {
        char *branch = exec("/usr/bin/git branch "
                            "--show-current 2>/dev/null");
        if (*branch == '\0') return branch;

        *(branch + strlen(branch) - 1) = '\0';

        if (strcmp(branch, "main") == 0) stpcpy(branch, ".");

        if (strcmp(branch, "master") == 0) stpcpy(branch, ":");

        return branch;
}

int main(void) {
        const char *device_name = getenv_checked("DEVICE");

        char battery[8 + 3 + 1] = "";
        get_battery_string(battery, device_name);

        if (!strcmp(device_name, "acer") || !strcmp(device_name, "mac"))
                device_name = "";

        time_t t = time(NULL);
        struct tm tm = *localtime(&t);

        char path[50];
        pwd(path);

        char *branch = get_git_branch();

        printf("\001\x1b[35m\002%s"
               "%s"
               "\001\x1b[33m\002%x%d"
               "\001\x1b[36m\002%s"
               "\001\x1b[32m\002%s\001\x1b[39m\002",
               device_name,
               battery,
               tm.tm_hour % 12,
               tm.tm_min,
               path,
               branch);

        free(branch);

        return 0;
}
