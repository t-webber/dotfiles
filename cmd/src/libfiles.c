#include "libfiles.h"
#include "lib.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

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
        static const char *const name##_EXT[] = {__VA_ARGS__};                 \
        static const size_t name##_LEN =                                       \
            sizeof(name##_EXT) / sizeof(name##_EXT[0]);                        \
        static bool name##_supported(const char *const extension) {            \
                return is_valid_extension(name##_EXT, extension, name##_LEN);  \
        }

extensions(timg, "jpg", "jpeg", "png", "gif", "tiff", "tif", "bmp", "webp",
           "jp2", "jpx", "heif", "heic", "avif", "svg")

    extensions(mpv,
               // Videos
               "mp4", "mkv", "avi", "mov", "wmv", "flv", "webm", "mpeg", "mpg",
               "3gp", "m4v", "ts", "vob", "rm", "rmvb", "ogv", "mts", "m2ts",
               "divx", "xvid", "f4v", "mxf", "y4m",

               // Audios
               "mp3", "aac", "wav", "flac", "ogg", "opus", "m4a", "alac", "wma",
               "ac3", "eac3", "dts", "dtsma", "truehd", "amr", "ra", "ape",

               // Subtitles
               "srt", "ass", "ssa", "vtt", "sub", "idx", "mpl", "txt")

        extensions(sxiv, "bmp", "jpg", "jpeg", "png", "gif", "tiff", "tif",
                   "xpm", "pcx", "tga", "pbm", "pgm", "ppm", "webp", "ico",
                   "heif", "heic", "psd", "hdr", "exr")

            int exec_open_file(const char *const filename,
                               const char *const extension, const bool is_open,
                               const bool is_kitty, const bool is_verbose) {
        if (is_kitty) {
                if (timg_supported(extension))
                        return execlp("timg", "timg", filename, NULL);

                if (!strcmp(extension, "pdf"))
                        return execlp("tdf", "tdf", filename, NULL);

                if (mpv_supported(extension))
                        return execlp("mpv", "mpv", "--vo=kitty", filename,
                                      NULL);
        }

        if (is_open) {
                if (!strcmp(extension, "pdf"))
                        return execlp("zathura", "zathura", filename, NULL);

                if (mpv_supported(extension))
                        return execlp("mpv", "mpv", filename, NULL);

                if (sxiv_supported(extension))
                        return execlp("sxiv", "sxiv", filename, NULL);

                if (!strcmp(extension, "html"))
                        return execlp("brave", "brave", filename, NULL);

                return execlp("nvim", "nvim", filename, NULL);
        }
        if (timg_supported(extension))
                panic("Use o %s.\n", filename);

        const char *const program = is_verbose ? "bat" : "cat";
        return execlp(program, program, filename, NULL);
}
