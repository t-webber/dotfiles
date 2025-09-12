#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define size ":size=20"
static const char *background_color = "#000000";
static const char *border_color = "#ffffaa";
static const char *font_color = "#ffffaa";
static const char *font_pattern = "Hack Nerd Font" size;
static const unsigned line_spacing = 20;
static const unsigned padding = 15;

static const unsigned width = 450;
static const unsigned border_size = 1;
static const unsigned pos_x = 0;
static const unsigned pos_y = 0;

enum corners { TOP_LEFT, TOP_RIGHT, BOTTOM_LEFT, BOTTOM_RIGHT };
enum corners corner = BOTTOM_LEFT;

static unsigned duration = 5; /* in seconds */

#define DISMISS_BUTTON Button1
#define ACTION_BUTTON Button3

Display *display;
Window window;

static unsigned int get_max_len(char *string, XftFont *font, unsigned max_text_width) {
        unsigned eol = (unsigned)strlen(string);
        XGlyphInfo info;
        XftTextExtentsUtf8(display, font, (FcChar8 *)string, (int)eol, &info);

        if (info.width > max_text_width) {
                eol = max_text_width / (unsigned int)font->max_advance_width;
                info.width = 0;

                while (info.width < max_text_width) {
                        eol++;
                        XftTextExtentsUtf8(display, font, (FcChar8 *)string, (int)eol, &info);
                }

                eol--;
        }

        for (unsigned i = 0; i < eol; i++)
                if (string[i] == '\n') {
                        string[i] = ' ';
                        return ++i;
                }

        if (info.width <= max_text_width) return eol;

        unsigned temp = eol;

        while (string[eol] != ' ' && eol) --eol;

        if (eol == 0)
                return temp;
        else
                return ++eol;
}

static void expire(int sig) {
        XEvent event;
        event.type = ButtonPress;
        event.xbutton.button = (sig == SIGUSR2) ? (ACTION_BUTTON) : (DISMISS_BUTTON);
        XSendEvent(display, window, 0, 0, &event);
        XFlush(display);
}

static void display_notification(int argc, char *argv[]) {

        pid_t pid = fork();
        if (pid) return;

        int fd = open("/dev/null", O_RDWR);

        if (fd != -1) {
                dup2(fd, STDIN_FILENO);
                dup2(fd, STDOUT_FILENO);
                dup2(fd, STDERR_FILENO);
                close(fd);
        }

        struct sigaction act_expire, act_ignore;

        act_expire.sa_handler = expire;
        act_expire.sa_flags = SA_RESTART;
        sigemptyset(&act_expire.sa_mask);

        act_ignore.sa_handler = SIG_IGN;
        act_ignore.sa_flags = 0;
        sigemptyset(&act_ignore.sa_mask);

        sigaction(SIGALRM, &act_expire, 0);
        sigaction(SIGTERM, &act_expire, 0);
        sigaction(SIGINT, &act_expire, 0);

        sigaction(SIGUSR1, &act_ignore, 0);
        sigaction(SIGUSR2, &act_ignore, 0);

        if (!(display = XOpenDisplay(0))) exit(1);

        int screen = DefaultScreen(display);
        Visual *visual = DefaultVisual(display, screen);
        Colormap colormap = DefaultColormap(display, screen);

        unsigned screen_width = DisplayWidth(display, screen);
        unsigned screen_height = DisplayHeight(display, screen);

        XSetWindowAttributes attributes;
        attributes.override_redirect = True;
        XftColor color;
        XftColorAllocName(display, visual, colormap, background_color, &color);
        attributes.background_pixel = color.pixel;
        XftColorAllocName(display, visual, colormap, border_color, &color);
        attributes.border_pixel = color.pixel;

        unsigned num_of_lines = 0;
        unsigned max_text_width = width - 2 * padding;
        unsigned lines_size = 5;
        char **lines = malloc(lines_size * sizeof(char *));
        if (!lines) exit(1);

        XftFont *font = XftFontOpenName(display, screen, font_pattern);

        unsigned max_rendered_line_width = 0;

        for (int i = 1; i < argc; i++) {
                for (unsigned eol = get_max_len(argv[i], font, max_text_width); eol; argv[i] += eol,
                              num_of_lines++,
                              eol = get_max_len(argv[i], font, max_text_width)) {
                        if (lines_size <= num_of_lines) {
                                lines = realloc(lines, (lines_size += 5) * sizeof(char *));
                                if (!lines) exit(1);
                        }

                        lines[num_of_lines] = malloc((eol + 1) * sizeof(char));
                        if (!lines[num_of_lines]) exit(1);

                        strncpy(lines[num_of_lines], argv[i], eol);
                        lines[num_of_lines][eol] = '\0';

                        XGlyphInfo extents;
                        XftTextExtentsUtf8(display,
                                           font,
                                           (FcChar8 *)lines[num_of_lines],
                                           (int)eol,
                                           &extents);
                        if (extents.width > max_rendered_line_width) {
                                max_rendered_line_width = extents.width;
                        }
                }
        }

        unsigned dynamic_width = max_rendered_line_width + 2 * padding;

        unsigned x = pos_x;
        unsigned y = pos_y;
        unsigned text_height = (unsigned int)font->ascent - (unsigned int)font->descent;
        unsigned height
            = (num_of_lines - 1) * line_spacing + num_of_lines * text_height + 2 * padding;

        if (corner == TOP_RIGHT || corner == BOTTOM_RIGHT)
                x = screen_width - dynamic_width - border_size * 2 - pos_x;

        if (corner == BOTTOM_LEFT || corner == BOTTOM_RIGHT)
                y = screen_height - height - border_size * 2 - pos_y;

        window = XCreateWindow(display,
                               RootWindow(display, screen),
                               (int)x,
                               (int)y,
                               dynamic_width,
                               (unsigned int)height,
                               border_size,
                               DefaultDepth(display, screen),
                               CopyFromParent,
                               visual,
                               CWOverrideRedirect | CWBackPixel | CWBorderPixel,
                               &attributes);

        XftDraw *draw = XftDrawCreate(display, window, visual, colormap);
        XftColorAllocName(display, visual, colormap, font_color, &color);

        XSelectInput(display, window, ExposureMask | ButtonPress);
        XMapWindow(display, window);

        sem_t *mutex = sem_open("/herbe", O_CREAT, 0644, 1);
        //         sem_wait(mutex);

        sigaction(SIGUSR1, &act_expire, 0);
        sigaction(SIGUSR2, &act_expire, 0);

        if (duration != 0) alarm(duration);

        for (;;) {
                XEvent event;
                XNextEvent(display, &event);

                if (event.type == Expose) {
                        XClearWindow(display, window);
                        for (unsigned i = 0; i < num_of_lines; i++)
                                XftDrawStringUtf8(
                                    draw,
                                    &color,
                                    font,
                                    padding,
                                    (int)(line_spacing * i + text_height * (i + 1) + padding),
                                    (FcChar8 *)lines[i],
                                    (int)strlen(lines[i]));
                } else if (event.type == ButtonPress
                           && (event.xbutton.button == DISMISS_BUTTON
                               || event.xbutton.button == ACTION_BUTTON))
                        break;
        }

        sem_post(mutex);
        sem_close(mutex);

        for (unsigned i = 0; i < num_of_lines; i++) free(lines[i]);

        free(lines);
        XftDrawDestroy(draw);
        XftColorFree(display, visual, colormap, &color);
        XftFontClose(display, font);
        XCloseDisplay(display);
}

int main(int argc, char *argv[]) {

        char **lines = malloc((size_t)argc * sizeof(char *));
        int lines_index = 0;
        lines[lines_index++] = NULL;

        for (int i = 1; i < argc; ++i) {
                if (!strcmp(argv[i], "em:"))
                        font_pattern = "JoyPixels" size;
                else if (!strcmp(argv[i], "sa:"))
                        font_pattern = "Symbola" size;
                else if (!strncmp(argv[i], "bg:", 3))
                        background_color = argv[i] + 3;
                else if (!strncmp(argv[i], "fg:", 3))
                        font_color = argv[i] + 3;
                else if (!strncmp(argv[i], "ft:", 3))
                        font_pattern = argv[i] + 3;
                else if (!strncmp(argv[i], "br:", 3))
                        border_color = argv[i] + 3;
                else if (!strncmp(argv[i], "ti:", 3)) {
                        unsigned dur = (unsigned)atoi(argv[i] + 3);
                        if (dur) duration = dur;

                } else {
                        lines[lines_index++] = argv[i];
                }
        }

        if (lines_index == 1) {
#define sep "\n    "
                const char *usage_const
                    = sep "<first-line>" sep "[<other-lines>...]" sep "[fg:<fg-colour>]" sep
                          "[bg:<bg-colour>]" sep "[ft:<text-font>]" sep "[br:<border-colour>]" sep
                          "[em:]" sep "[sa:]"
                          "\n";
                char *usage = malloc(strlen(argv[0]) + strlen(usage_const));
                stpcpy(stpcpy(usage, argv[0]), usage_const);
                fprintf(stderr, "%s\n", usage);
                lines[lines_index++] = usage;
        }

        display_notification(lines_index, lines);
}
