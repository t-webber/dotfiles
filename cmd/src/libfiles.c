#include "libfiles.h"
#include "lib.h"
#include "libexec.h"
#include "libos.h"
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

__attribute_pure__ __wur __nonnull() const
    char *get_filename_extension(const_str filename) {
        const size_t len = strlen(filename);
        const char *end = filename + len;
        while (end != filename && *end != '.') --end;
        if (*end == '.') return ++end;
        return end;
}

__wur __nonnull() __attribute_pure__
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
        __wur __nonnull() __attribute_pure__ static bool name##_supported(     \
            const_str extension) {                                             \
                return is_valid_extension(name##_EXT, extension, name##_LEN);  \
        }

// clang-format off
extensions(timg,"jpg","jpeg","png","gif","tiff","tif","bmp","webp","jp2","jpx","heif","heic","avif","svg")

extensions(mpv,"mp4","mkv","avi","mov","wmv","flv","webm","mpeg","mpg","3gp","m4v","vob","rm","rmvb","ogv","mts","m2ts","divx","xvid","f4v","mxf","y4m","mp3","aac","wav","flac","ogg","opus","m4a","alac","wma","ac3","eac3","dts","dtsma","truehd","amr","ra","ape","srt","ass","ssa","vtt","sub","idx","mpl")

extensions(sxiv,"bmp","jpg","jpeg","png","gif","tiff","tif","xpm","pcx","tga","pbm","pgm","ppm","webp","ico","heif","heic","psd","hdr","exr")
    // clang-format on

    static void send_alacritty_config(const_str font_size) {
        forked_exldn("alacritty", "msg", "config", font_size);
}

static sig_atomic_t sigflag = 0;

static void sighandler(void) {
        printf("pressed ctrl+c\n");
        sigflag = 1;
}

_Noreturn __nonnull() static void dezoom_and_run_alacritty(const_str filename,
                                                           const bool is_mpv) {
        signal(SIGINT, (void (*)(int))sighandler);
        send_alacritty_config("font.size = 2");

        if (is_mpv) {
                forked_exldn("mpv",
                             "--vo=tct",
                             "--msg-level=all=no",
                             "--log-file=mpv.log",
                             filename,
                             "2>mpv.error");
        } else {
                forked_exldn("timg", filename);
        }

        if (!is_mpv)
                while (sigflag == 0) pause();

        send_alacritty_config("font.size = 6");

        printf("\033[2J\033[H");
        fflush(stdout);

        exit(0);
}

_Noreturn __nonnull() static void display_todo_file(const_str filename) {
        FILE *fd = fopen_checked(filename, "r");
        char line[1024];
        while (fgets(line, 1024, fd)) {
                const char *colour = "\033[0m";
                if (starts_with_const(line, "- [x] ")) { colour = "\033[32m"; }
                if (starts_with_const(line, "- [ ] ")) { colour = "\033[31m"; }
                if (starts_with_const(line, "- [.] ")) { colour = "\033[33m"; }
                if (starts_with_const(line, "#")) { colour = "\033[35m"; }

                printf("%s%s", colour, line);
        }
        exit(0);
}

_Noreturn __nonnull() void exec_open_file(const_str filename,
                                          const display_type ty) {

        const_str extension = get_filename_extension(filename);
        terminal_app terminal = what_terminal();

        if (terminal == TERMINAL_KITTY) {
                if (timg_supported(extension)) exldn("timg", filename);

                if (!strcmp(extension, "pdf")) exldn("tdf", filename);

                if (mpv_supported(extension))
                        exldn("mpv", "--vo=kitty", filename);
        }

        if (ty == DISPLAY_OPEN) {
                if (!strcmp(extension, "pdf")) {
                        if (is_file("/usr/bin/zathura"))
                                exldn("zathura", filename);
                        exldn("brave", filename);
                };

                if (mpv_supported(extension)) exldn("mpv", filename);

                if (sxiv_supported(extension)) exldn("sxiv", filename);

                if (!strcmp(extension, "html")) exldn("brave", filename);

                exldn("nvim", filename);
        }

        if (mpv_supported(extension) && terminal == TERMINAL_ALACRITTY)
                dezoom_and_run_alacritty(filename, true);

        if (timg_supported(extension))
                dezoom_and_run_alacritty(filename, false);

        if (ty == DISPLAY_VERBOSE) {
                if (!strcmp(extension, "todo")) { display_todo_file(filename); }

                if (is_file_binary(filename)) { exldn("file", filename); }

                exldn("bat", filename);
        }

        exldn("cat", filename);
}
