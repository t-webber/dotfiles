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

#define extensions(name, ...)                                                  \
        static const char *const name[] = {__VA_ARGS__};                       \
        static const size_t name##_LEN = sizeof(name) / sizeof(name[0])

extensions(TIMG_EXTENSIONS, "jpg", "jpeg", "png", "gif", "tiff", "tif", "bmp",
           "webp", "jp2", "jpx", "heif", "heic", "avif", "svg", "mp4", "webm",
           "mp3", "wav", );

extensions(BRAVE_EXTENSIONS, "pdf", "html");

const char *get_filename_extension(const char *const filename,
                                   const size_t len);

bool is_valid_extension(const char *const *const supported,
                        const char *const extension,
                        const size_t nb_extensions);

bool has_slash(const char *const arg, size_t *len, const char **const position);
