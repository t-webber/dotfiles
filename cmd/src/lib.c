#include "lib.h"
#include "libexec.h"
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

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

__nonnull() void store_usage(Args argv) {
        const_str logs = getenv_checked("LOGS");
        const size_t logs_len = strlen(logs);
        str path
            = malloc(sizeof(char) * (logs_len + sizeof("/aliases.log") + 1));
        char *end = stpcpy(path, logs);
        stpcpy(end, "/aliases.log");
        FILE *fd = fopen_checked(path, "a");
        fprintf(fd, "%s\n", argv[0]);
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

__nonnull() __wur FILE *fopen_checked(const_str file_name, const_str mode) {
        FILE *f = fopen(file_name, mode);
        if (f == NULL) epanic("Failed to open file %s", file_name);
        return f;
}

__wur __nonnull() DIR *opendir_checked(const_str dirname) {
        DIR *dirp = opendir(dirname);
        if (dirp == NULL) epanic("Failed to open dir %s", dirname);
        return dirp;
}

__nonnull() void closedir_checked(DIR *dirp) {
        if (closedir(dirp)) epanic("Failed to close dir");
}

__wur __nonnull() FILE *popen_checked(const_str command) {
        FILE *f = popen(command, "r");
        if (f == NULL) epanic("Failed to run %s", command);
        return f;
}

__wur __nonnull() const_var_str getenv_checked(const_str var) {
        const_str value = getenv(var);
        if (value == NULL) upanic("Env var %s not defined.", var);
        return value;
}

__nonnull() _Noreturn void exl_notif(const_str message) {
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
}

__wur char *get_battery_level(void) {
        const_str device = getenv("DEVICE");
        if (!device) return NULL;
        if (!strcmp(device, "acer")) {
                FILE *fd
                    = fopen_checked("/sys/class/power_supply/BAT1/capacity",
                                    "r");
                char *content = malloc(5 * sizeof(char));
                fgets(content, 5, fd);
                content[strlen(content) - 1] = '\0';
                return content;
        }
        if (!strcmp(device, "mac")) {
                read_simple_exl1(8, buffer, "pwmcharge");
                return buffer;
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
                if (!strcmp(content, "Full")) {
                        free(content);
                        return BATTERY_STATUS_FULL;
                }
                upanic("Invalid battery status :%s:", content);
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
                upanic("Invalid battery status :%s:", content);
        }
        return BATTERY_STATUS_UNKNOWN;
}

_Noreturn void exl_err_notif(void) {
        exldn("notify-send", "-u", "critical", "error");
}

void clear(void) {
        printf("\033c");
        fflush(stdout);
}

_Noreturn __nonnull() void exl_err_notif_msg(const_str err_msg) {
        exldn("notify-send", "-u", "critical", err_msg);
}

__wur __attribute_const__ size_t max(const size_t a, const size_t b) {
        return a > b ? a : b;
}

__nonnull() void chdir_checked(const_str path) {
        if (chdir(path)) { epanic("Failed to change process cwd to %s", path) };
}
