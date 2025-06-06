/// Contains the common code for multiple programs.

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NEW_STRING(str) {str, sizeof(str) - 1}

typedef struct {
        const char *const value;
        const size_t length;
} String;

String get_env_subpath(const String subpath, const char *const var);

#define panic(...)                                                             \
        {                                                                      \
                fprintf(stderr, __VA_ARGS__);                                  \
                exit(1);                                                       \
        }

bool is_verbose(const char *const program_name, const char *const normal_name,
                const char *const verbose_name);

const char *argv_one_filename(const int argc, const char *const *const argv);

const char *get_filename_extension(const char *const filename,
                                   const size_t len);

bool has_timg_support(const char *const extension);
bool has_brave_support(const char *const extension);

bool has_slash(const char *const arg, size_t *len, const char **const position);
