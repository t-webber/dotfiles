#include "lib.h"
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
        SHORT(path, alias, strcmp(getenv(var), pwd) == 0);

static char *exec(const char *const command) {
        char *output = malloc(200 * sizeof(char));
        *output = '\0';
        FILE *f = popen(command, "r");
        fgets(output, 199, f);
        pclose(f);
        return output;
}

static void pwd(char *const path, const bool is_acer) {
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

        if (is_acer) {
                SHORT(path, "/", strcmp(pwd, "/") == 0);
                SHORTP(path, pwd, "HOME", "~");
                SHORTP(path, pwd, "FILES", "f");
                SHORTP(path, pwd, "CMD", "c");
                SHORTP(path, pwd, "APPS", "a");
                SHORTP(path, pwd, "DEV", "d");
                SHORTP(path, pwd, "DATA", "t");
                SHORTP(path, pwd, "DOT", ".");
                SHORTP(path, pwd, "DOT", ".");
        }

        while (*end != '/' && *end != '.')
                --end;

        if (end != pwd || *(end + 1) != '\0')
                ++end;

        stpcpy(path, end);

        free(pwd);
}

static char *get_battery_level(void) {
        FILE *fd = fopen("/sys/class/power_supply/BAT1/capacity", "r");
        char *content = malloc(4 * sizeof(char));
        fgets(content, 4, fd);
        return content;
}

static bool is_battery_charging(void) {
        FILE *fd = fopen("/sys/class/power_supply/BAT1/status", "r");
        char content[10];
        fgets(content, 10, fd);
        content[strlen(content) - 1] = '\0';
        return !strcmp(content, "Charging");
}

static void wait_and_suspend(void) {
        pid_t pid = fork();

        if (pid < 0) {
                panic("suspend failed (0), but plug it in.\n");
        }

        if (pid == 0) {
                printf("\033[31mBattery running out. Plug your "
                       "computer.\033[0m\n");
                setsid();
                sleep(60);
                if (is_battery_charging())
                        return;
                execl("/bin/sudo", "sudo", "systemctl", "suspend", NULL);
                panic("suspend failed (1), but plug it in\n");
        }

        int status;
        waitpid(pid, &status, 0);
        if (!WIFEXITED(status))
                panic("Failed to execute in fork\n")
}

static void get_battery_string(char *const battery, const bool is_acer) {
        if (!is_acer) {
                char *end = stpcpy(battery, "\001\x1b[31m\002");
                stpcpy(end, "");
                return;
        }

        char *const level = get_battery_level();
        const int level_value = atoi(level);
        if (level_value >= 79)
                return;

        const bool is_charging = is_battery_charging();

        char *end;
        if (is_charging) {
                end = stpcpy(battery, "\001\x1b[32m\002");
        } else {
                end = stpcpy(battery, "\001\x1b[31m\002");
        }

        if ((level_value <= 6 && !is_charging) || level_value <= 3)
                wait_and_suspend();

        *end++ = level[0];
        *end = '\0';
        return;
}

static char *get_git_branch(void) {
        char *branch = exec("/usr/bin/git branch --show-current 2>/dev/null");
        if (*branch == '\0')
                return branch;

        *(branch + strlen(branch) - 1) = '\0';

        if (strcmp(branch, "main ") == 0)
                stpcpy(branch, ".");
        if (strcmp(branch, "master ") == 0)
                stpcpy(branch, ":");

        //         if (strchr(branch, '-')) {
        //                 char *reader = branch;
        //                 char *writer = branch;
        //                 size_t written = 0;
        //                 while (*reader) {
        // 			if (written <=
        //                 }
        //         }

        return branch;
}

int main(void) {
        const char *device_name = getenv("DEVICE");
        if (device_name == NULL)
                device_name = "";

        bool is_acer = device_name != NULL && !strcmp(device_name, "acer");
        if (is_acer)
                device_name = "";

        char battery[8 + 3 + 1] = "";
        get_battery_string(battery, is_acer);

        time_t t = time(NULL);
        struct tm tm = *localtime(&t);

        char path[50];
        pwd(path, is_acer);

        char *branch = get_git_branch();

        printf("\001\x1b[35m\002%s"
               "%s"
               "\001\x1b[33m\002%x%d"
               "\001\x1b[36m\002%s"
               "\001\x1b[32m\002%s\001\x1b[39m\002",
               device_name, battery, tm.tm_hour % 12, tm.tm_min, path, branch);

        free(branch);

        return 0;
}
