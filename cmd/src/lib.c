#include "lib.h"
#include "libexec.h"
#include "libos.h"

#include <dirent.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <time.h>
__nonnull() void print_inline_array(const char *const *const array) {
        for (const char *const *arg = array; *arg; ++arg) {
                printf("%s ", *arg);
        }
        printf("\n");
}

void print_inline_variadic(const_str first, ...) {
        va_list args;
        const char *current = first;
        va_start(args, first);
        while (current != NULL) {
                printf("%s ", current);
                current = va_arg(args, const char *);
        }
        printf("\n");
}

__nonnull() __wur String get_env_subpath(const String subpath, const_str var) {
        const_str cmd_path = getenv_checked(var);

        const size_t cmd_path_len = strlen(cmd_path);
        const size_t fullpath_len = cmd_path_len + subpath.length;

        str fullpath = malloc(sizeof(char) * fullpath_len);
        char *end = stpcpy(fullpath, cmd_path);
        strcpy(end, subpath.value);

        const String fullpath_str = {.value = fullpath, .length = fullpath_len};
        return fullpath_str;
}

__nonnull() __wur bool is_verbose(const_str program_name,
                                  const_str normal_name,
                                  const_str verbose_name) {
        if (!strcmp(program_name, normal_name))
                return false;
        else if (!strcmp(program_name, verbose_name))
                return true;
        else
                upanic("Invalid executable name. Please name it `%s` or `%s`",
                       normal_name,
                       verbose_name);
}

__nonnull() __wur const
    char *argv_one_filename(const int argc, const_str *const argv) {

        if (argc == 1)
                return ".";
        else if (argc == 2)
                return argv[1];
        else
                upanic("Too many arguments. Usage: %s [<filename>]", argv[0]);
}

__nonnull() __wur __attribute_pure__
    bool has_slash(const_str arg, size_t *len, const char **const position) {
        const char *end = arg;
        bool res = false;
        for (; *end != '\0'; ++end)
                if (*end == '/') {
                        res = true;
                        *position = (end + 1);
                }
        *len = (size_t)(end - arg);
        return res;
}

__nonnull() void store_usage(const_str prog_name,
                             const_str arg,
                             const bool is_alias) {
        const_str logs = getenv_checked("LOGS");
        char path[512];
        sprintf(path, "%s/aliases.log", logs);
        FILE *fd = fopen_checked(path, "a");
        fprintf(fd, "%s %s %d\n", prog_name, arg, is_alias);
        fclose(fd);
}

__attribute_pure__ __wur __nonnull() size_t utf8_strlen(const_str s) {
        size_t len = 0;
        const char *reader = s;
        while (*reader) {
                if ((*reader & 0xC0) != 0x80) len++;
                reader++;
        }
        return len;
}

__nonnull() void setenv_checked(const_str var, const_str val) {
        if (setenv(var, val, !0) == 0) return;
        epanic("Setting var env %s to %s failed", var, val);
}

__wur __nonnull() const_var_str getenv_checked(const_str var) {
        const_str value = getenv(var);
        if (value == NULL) upanic("Env var %s not defined.", var);
        return value;
}

__wur char *get_battery_level(void) {
        const_str device = getenv("DEVICE");
        if (!device) return NULL;
        if (!strcmp(device, "acer")) {
                FILE *fd
                    = fopen_checked("/sys/class/power_supply/BAT1/capacity",
                                    "r");
                char *content = malloc(8 * sizeof(char));
                fgets(content, 8, fd);
                content[strlen(content) - 1] = '\0';
                return content;
        }
        if (!strcmp(device, "mac")) {
                read_simple_exl1(8, content, "pwmcharge");
                content[strlen(content) - 1] = '\0';
                return content;
        }
        return NULL;
}

__wur battery_status get_battery_status(void) {
        const_str device = getenv("DEVICE");
        if (!device) return BATTERY_STATUS_UNKNOWN;
        if (!strcmp(device, "acer")) {
                FILE *fd
                    = fopen_checked("/sys/class/power_supply/BAT1/status", "r");
                char *content = malloc(32 * sizeof(char));
                fgets(content, 32, fd);
                content[strlen(content) - 1] = '\0';
                if (!strcmp(content, "Charging")) {
                        free(content);
                        return BATTERY_STATUS_CHARGING;
                }
                if (!strcmp(content, "Discharging")) {
                        free(content);
                        return BATTERY_STATUS_DISCHARGING;
                }
                if (!strcmp(content, "Full")
                    || !strcmp(content, "Not charging")) {
                        free(content);
                        return BATTERY_STATUS_FULL;
                }
                upanic("Invalid acer battery status :%s:", content);
        }
        if (!strcmp(device, "mac")) {
                read_simple_exl1(8, content, "pwmstatus");
                content[strlen(content) - 1] = '\0';
                if (!strcmp(content, "Yes")) {
                        free(content);
                        return BATTERY_STATUS_CHARGING;
                };
                if (!strcmp(content, "No")) {
                        free(content);
                        return BATTERY_STATUS_DISCHARGING;
                };
                upanic("Invalid mac battery status :%s:", content);
        }
        return BATTERY_STATUS_UNKNOWN;
}

__wur __attribute_const__ size_t max(const size_t a, const size_t b) {
        return a > b ? a : b;
}

void slp(const long secs, const long nanos) {
        struct timespec ts;
        ts.tv_sec = secs;
        ts.tv_nsec = nanos;
        nanosleep(&ts, NULL);
}
