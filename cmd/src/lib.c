#include "lib.h"
#include <stdio.h>

String get_env_subpath(const String subpath, const char *const var) {
        const char *const cmd_path = getenv(var);

        const size_t cmd_path_len = strlen(cmd_path);
        const size_t fullpath_len = cmd_path_len + subpath.length;

        char *const fullpath = malloc(sizeof(char) * fullpath_len);
        char *end = stpcpy(fullpath, cmd_path);
        strcpy(end, subpath.value);

        const String fullpath_str = {.value = fullpath, .length = fullpath_len};
        return fullpath_str;
}

bool is_verbose(const char *const program_name, const char *const normal_name,
                const char *const verbose_name) {
        if (!strcmp(program_name, normal_name))
                return false;
        else if (!strcmp(program_name, verbose_name))
                return true;
        else
                panic("Invalid executable name. Please name it `%s` or `%s`.\n",
                      normal_name, verbose_name);
}

const char *argv_one_filename(const int argc, const char *const *const argv) {

        if (argc == 1)
                return ".";
        else if (argc == 2)
                return argv[1];
        else
                panic("Too many arguments. Usage: %s [<filename>].\n", argv[0]);
}

bool has_slash(const char *const arg, size_t *len,

               const char **const position) {
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
