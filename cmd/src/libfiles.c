#include "libfiles.h"
#include "lib.h"
#include "libexec.h"
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

const char *get_filename_extension(const_str filename, const size_t len) {
        const char *end = filename + len;
        while (end != filename && *end != '.') --end;
        if (*end == '.') return ++end;
        return end;
}

static bool is_valid_extension(const_str *const supported,
                               const_str extension,
                               const size_t nb_extensions) {
        for (size_t i = 0; i < nb_extensions; ++i)
                if (strcmp(extension, supported[i]) == 0) return true;

        return false;
}

#define extensions(name, ...)                                                  \
        static const_str name##_EXT[] = {__VA_ARGS__};                         \
        static const size_t name##_LEN                                         \
            = sizeof(name##_EXT) / sizeof(name##_EXT[0]);                      \
        static bool name##_supported(const_str extension) {                    \
                return is_valid_extension(name##_EXT, extension, name##_LEN);  \
        }                                                                      \
        void *x

extensions(timg,
           "jpg",
           "jpeg",
           "png",
           "gif",
           "tiff",
           "tif",
           "bmp",
           "webp",
           "jp2",
           "jpx",
           "heif",
           "heic",
           "avif",
           "svg");

extensions(mpv,
           // Videos
           "mp4",
           "mkv",
           "avi",
           "mov",
           "wmv",
           "flv",
           "webm",
           "mpeg",
           "mpg",
           "3gp",
           "m4v",
           "vob",
           "rm",
           "rmvb",
           "ogv",
           "mts",
           "m2ts",
           "divx",
           "xvid",
           "f4v",
           "mxf",
           "y4m",

           // Audios
           "mp3",
           "aac",
           "wav",
           "flac",
           "ogg",
           "opus",
           "m4a",
           "alac",
           "wma",
           "ac3",
           "eac3",
           "dts",
           "dtsma",
           "truehd",
           "amr",
           "ra",
           "ape",

           // Subtitles
           "srt",
           "ass",
           "ssa",
           "vtt",
           "sub",
           "idx",
           "mpl");

extensions(sxiv,
           "bmp",
           "jpg",
           "jpeg",
           "png",
           "gif",
           "tiff",
           "tif",
           "xpm",
           "pcx",
           "tga",
           "pbm",
           "pgm",
           "ppm",
           "webp",
           "ico",
           "heif",
           "heic",
           "psd",
           "hdr",
           "exr");

static void send_alacritty_config(const_str font_size) {
        forked_exl("alacritty", "alacritty", "msg", "config", font_size, NULL);
}

static sig_atomic_t sigflag = 0;

static void sighandler(void) {
        printf("pressed ctrl+c\n");
        sigflag = 1;
}

static int dezoom_and_run_alacritty(const_str filename, const bool is_mpv) {
        signal(SIGINT, (void (*)(int))sighandler);
        send_alacritty_config("font.size = 2");

        if (is_mpv) {
                forked_exl("mpv",
                           "mpv",
                           "--vo=tct",
                           "--msg-level=all=no",
                           "--log-file=mpv.log",
                           filename,
                           "2>mpv.error",
                           NULL);
        } else {
                forked_exl("timg", "timg", filename, NULL);
        }

        if (!is_mpv)
                while (sigflag == 0) pause();

        send_alacritty_config("font.size = 6");

        printf("\033[2J\033[H");
        fflush(stdout);

        exit(0);
}

int exec_open_file(const_str filename,
                   const_str extension,
                   const bool is_open,
                   const bool is_kitty,
                   const bool is_verbose) {
        if (is_kitty) {
                if (timg_supported(extension))
                        exl("timg", "timg", filename, NULL);

                if (!strcmp(extension, "pdf"))
                        exl("tdf", "tdf", filename, NULL);

                if (mpv_supported(extension))
                        exl("mpv", "mpv", "--vo=kitty", filename, NULL);
        }

        if (is_open) {
                if (!strcmp(extension, "pdf"))
                        exl("zathura", "zathura", filename, NULL);

                if (mpv_supported(extension)) exl("mpv", "mpv", filename, NULL);

                if (sxiv_supported(extension))
                        exl("sxiv", "sxiv", filename, NULL);

                if (!strcmp(extension, "html"))
                        exl("brave", "brave", filename, NULL);

                exl("nvim", "nvim", filename, NULL);
        }

        if (mpv_supported(extension)) dezoom_and_run_alacritty(filename, true);

        if (timg_supported(extension))
                dezoom_and_run_alacritty(filename, false);

        const_str program = is_verbose ? "bat" : "cat";
        exl(program, program, filename, NULL);
}
