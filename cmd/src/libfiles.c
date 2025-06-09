#include "libfiles.h"
#include "lib.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
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
               "srt", "ass", "ssa", "vtt", "sub", "idx", "mpl")

        extensions(sxiv, "bmp", "jpg", "jpeg", "png", "gif", "tiff", "tif",
                   "xpm", "pcx", "tga", "pbm", "pgm", "ppm", "webp", "ico",
                   "heif", "heic", "psd", "hdr", "exr")

            static void set_alacritty_zoom(const char *const config_path,
                                           const char zoom) {
        FILE *fd = fopen(config_path, "r");
        if (!fd)
                panic("No config found at %s. Please create an alacritty "
                      "config with default font size.\n",
                      config_path);

        char buffer[1024];
        size_t n = fread(buffer, 1, sizeof(buffer) - 1, fd);
        fclose(fd);

        buffer[n] = '\0';

        char *const pos = strstr(buffer, "size");
        if (!pos)
                panic("Missing size parameter in alacritty config.\n");

        *(pos + 4) = '=';
        *(pos + 5) = zoom;
        for (int i = 6; *(pos + i) != '\0' && *(pos + i) != '\n'; ++i)
                *(pos + i) = ' ';

        fd = fopen(config_path, "w");
        fprintf(fd, "%s", buffer);
        fclose(fd);
}

static sig_atomic_t sigflag = 0;

static void sighandler(void) {
        printf("pressed ctrl+c\n");
        sigflag = 1;
}

static int dezoom_and_run_alacritty(const char *const filename,
                                    const bool is_mpv) {
        signal(SIGINT, (void (*)(int))sighandler);
        char *const config_path = malloc(128);
        stpcpy(stpcpy(config_path, getenv("XDG_CONFIG_HOME")),
               "/alacritty/alacritty.toml");
        set_alacritty_zoom(config_path, '1');
        pid_t pid = fork();
        if (pid < 0)
                panic("Failed to fork.\n");
        if (pid == 0) {
                int res;
                if (is_mpv) {
                        int null = open("/dev/null", O_WRONLY);
                        dup2(null, 2);
                        close(null);
                        res = execlp("mpv", "mpv", "--vo=tct", "2>/dev/null",
                                     filename, NULL);
                } else {
                        res = execlp("timg", "timg", filename, NULL);
                }
                panic("Failed to execute: exit code %d: %d.\n", res, errno)
        }
        int stat;
        waitpid(pid, &stat, 0);
        if (!WIFEXITED(stat))
                panic("Failed to execute.\n");

        printf("here\n");

        if (!is_mpv)
                while (sigflag == 0)
                        pause();

        set_alacritty_zoom(config_path, '6');
        printf("\033[2J\033[H");
        fflush(stdout);

        exit(0);
}

int exec_open_file(const char *const filename, const char *const extension,
                   const bool is_open, const bool is_kitty,
                   const bool is_verbose) {
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

        if (mpv_supported(extension))
                dezoom_and_run_alacritty(filename, true);

        if (timg_supported(extension))
                dezoom_and_run_alacritty(filename, false);

        const char *const program = is_verbose ? "bat" : "cat";
        return execlp(program, program, filename, NULL);
}
