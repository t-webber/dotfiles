#include "lib.h"
#include <stdio.h>

String get_env_subpath(const String subpath, const char *const var) {
        const char *const cmd_path = getenv(var);

        const size_t cmd_path_len = strlen(cmd_path);
        const size_t fullpath_len = cmd_path_len + subpath.length;

        char *const fullpath = (char *)malloc(sizeof(char) * fullpath_len);
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

const char *get_filename_extension(const char *const filename,
                                   const size_t len) {
        const char *end = filename + len;
        while (end != filename && *end != '.')
                --end;
        if (*end == '.')
                return ++end;
        return end;
}

static bool is_valid_extension(const char *const *const supported,
                               const char *const extension,
                               const size_t nb_extensions) {
        for (size_t i = 0; i < nb_extensions; ++i)
                if (strcmp(extension, supported[i]) == 0)
                        return true;

        return false;
}

#define extensions(name, ...)                                                  \
        static const char *const name[] = {__VA_ARGS__};                       \
        static const size_t name##_LEN = sizeof(name) / sizeof(name[0])

extensions(TIMG_EXTENSIONS, "jpg", "jpeg", "png", "gif", "tiff", "tif", "bmp",
           "webp", "jp2", "jpx", "heif", "heic", "avif", "svg", "mp4", "webm",
           "mp3", "wav", );

bool has_timg_support(const char *const extension) {
        return is_valid_extension(TIMG_EXTENSIONS, extension,
                                  TIMG_EXTENSIONS_LEN);
}

extensions(BRAVE_EXTENSIONS, "pdf", "html");

bool has_brave_support(const char *const extension) {
        return is_valid_extension(BRAVE_EXTENSIONS, extension,
                                  BRAVE_EXTENSIONS_LEN);
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
