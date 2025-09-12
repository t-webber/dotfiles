#include "lib.h"
#include "libexec.h"
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

String get_env_subpath(const String subpath, const_str var) {
        const_str cmd_path = getenv_checked(var);

        const size_t cmd_path_len = strlen(cmd_path);
        const size_t fullpath_len = cmd_path_len + subpath.length;

        str fullpath = malloc(sizeof(char) * fullpath_len);
        char *end = stpcpy(fullpath, cmd_path);
        strcpy(end, subpath.value);

        const String fullpath_str = {.value = fullpath, .length = fullpath_len};
        return fullpath_str;
}

bool is_verbose(const_str program_name,
                const_str normal_name,
                const_str verbose_name) {
        if (!strcmp(program_name, normal_name))
                return false;
        else if (!strcmp(program_name, verbose_name))
                return true;
        else
                upanic("Invalid executable name. Please "
                       "name it `%s` or `%s`.",
                       normal_name,
                       verbose_name);
}

const char *argv_one_filename(const int argc, const_str *const argv) {

        if (argc == 1)
                return ".";
        else if (argc == 2)
                return argv[1];
        else
                upanic("Too many arguments. Usage: %s "
                       "[<filename>].",
                       argv[0]);
}

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

void store_usage(Args argv) {
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

size_t utf8_strlen(const_str s) {
        size_t len = 0;
        const char *reader = s;
        while (*reader) {
                if ((*reader & 0xC0) != 0x80) len++;
                reader++;
        }
        return len;
}

FILE *fopen_checked(const_str file_name, const_str mode) {
        FILE *f = fopen(file_name, mode);
        if (f == NULL) epanic("Failed to open file %s", file_name);
        return f;
}

DIR *opendir_checked(const_str file_name) {
        DIR *f = opendir(file_name);
        if (f == NULL) epanic("Failed to popen %s", file_name);
        return f;
}

FILE *popen_checked(const_str command) {
        FILE *f = popen(command, "r");
        if (f == NULL) epanic("Failed to run %s", command);
        return f;
}

const_var_str getenv_checked(const_str var) {
        const_str value = getenv(var);
        if (value == NULL) upanic("Env var %s not defined.", var);
        return value;
}

void exl_notif(const_str message) {
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

char *get_battery_level(void) {
        const_str device = getenv("DEVICE");
        if (!device || strcmp(device, "acer")) { return NULL; }
        FILE *fd = fopen_checked("/sys/class/power_supply/BAT1/capacity", "r");
        char *content = malloc(4 * sizeof(char));
        fgets(content, 4, fd);
        content[strlen(content) - 1] = '\0';
        return content;
}

char *get_battery_status(void) {
        const_str device = getenv("DEVICE");
        if (!device || strcmp(device, "acer")) { return NULL; }
        FILE *fd = fopen_checked("/sys/class/power_supply/BAT1/status", "r");
        char *content = malloc(32 * sizeof(char));
        fgets(content, 32, fd);
        content[strlen(content) - 1] = '\0';
        return content;
}

void exl_err_notif(void) {
        exldn("notify-send", "-u", "critical", "error");
}

void clear(void) {
        printf("\033c");
        fflush(stdout);
}

void exl_err_notif_msg(const_str err_msg) {
        exldn("notify-send", "-u", "critical", err_msg);
}
