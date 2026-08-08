#include "lib.h"
#include "libexec.h"
#include "libos.h"
#include <dirent.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

__nonnull() void print_inline_array(const char *const *const array) {
        for (const char *const *arg = array; *arg; ++arg) { printf("'%s' ", *arg); }
        printf("\n");
}

void print_inline_variadic(const_str first, ...) {
        va_list args;
        const char *current = first;
        va_start(args, first);
        while (current != NULL) {
                printf("'%s' ", current);
                current = va_arg(args, const char *);
        }
        printf("\n");
}

__nonnull() __wur
    bool is_verbose(const_str program_name, const_str normal_name, const_str verbose_name) {
        if (!strcmp(program_name, normal_name))
                return false;
        else if (!strcmp(program_name, verbose_name))
                return true;
        else if (getenv("DEBUG") == NULL) {
                upanic("Invalid executable name. Please name it `%s` or `%s`",
                       normal_name,
                       verbose_name);
        } else
                return false;
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

__nonnull() void store_usage(const_str prog_name, const_str arg, const bool is_alias) {
        (void)prog_name;
        (void)arg;
        (void)is_alias;
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

#define MAX_ENV 32
static const char *THIS_ENV_VARS[MAX_ENV];
static size_t env_index = 0;

void print_this_env(void) {
        for (size_t i = 0; i < env_index; ++i) {
                const_str env = THIS_ENV_VARS[i];
                const_str val = getenv_checked(env);
                printf("%s='%s' ", env, val);
        }
}

__nonnull() void setenv_checked(const_str var, const_str val) {
        if (setenv(var, val, !0)) epanic("Setting var env %s to %s failed", var, val);
        if (env_index + 1 == MAX_ENV)
                upanic("Too many environment variables set.") THIS_ENV_VARS[env_index++] = var;
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
                FILE *fd = fopen_checked("/sys/class/power_supply/BAT1/capacity", "r");
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
        FILE *fd = fopen("/sys/class/power_supply/BAT1/status", "r");
        if (fd) {
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
                if (!strcmp(content, "Full") || !strcmp(content, "Not charging")) {
                        free(content);
                        return BATTERY_STATUS_FULL;
                }
                upanic("Invalid acer battery status :%s:", content);
        }
        if (is_file("/Users")) {
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

__nonnull() void printn(const_str pattern, const size_t n) {
        for (size_t i = 0; i < n; ++i) { fputs(pattern, stdout); }
}

void slp(const long secs, const long nanos) {
        struct timespec ts;
        ts.tv_sec = secs;
        ts.tv_nsec = nanos;
        nanosleep(&ts, NULL);
}

__wur __attribute_const__ char *unsafe_const_cast(const char *comp) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
        return (char *)comp;
#pragma GCC diagnostic pop
}

void set_lht_level(const char *amount, const brightness_function f) {
        if (!amount) amount = "20";
        if (f == BRIGHTNESS_SET) {
                if (!fork_and_wait()) {
                        FILE *const devnull = fopen_checked("/dev/null", "w");
                        dup2(fileno(devnull), STDOUT_FILENO);
                        fclose(devnull);
                        exldn("sudo", "brightnessctl", "s", amount);
                }
                return;
        }
        char *const full = malloc(sizeof(char) * 16);
        sprintf(full, "%s%s", amount, f == BRIGHTNESS_UP ? "+" : "-");
        forked_exldn("sudo", "brightnessctl", "s", full);
}

__wur uint64_t since_unix(void) {
        static const uint64_t NS = 1;
        static const uint64_t SEC = 1000 * 1000 * 1000 * NS;
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        const uint64_t nsec = (uint64_t)ts.tv_nsec;
        const uint64_t sec = (uint64_t)ts.tv_sec;
        return (nsec * NS + sec * SEC);
}

_Noreturn __nonnull() void rn_file(const_str filename) {
        const_str waste = getenv_checked("WASTE");
        mkdir(waste, 0755);
        char path[256];
        snprintf(path, sizeof(path), "%s/%" PRIu64 "/", waste, since_unix());
        printf("%s => %s\n", filename, path);
        mkdir(path, 0755);
        exldn("mv", filename, path);
}
