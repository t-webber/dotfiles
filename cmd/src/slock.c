/*
This file was inspired by Slock from Suckless, and was heavily modified.

Original license:

MIT/X Consortium License

© 2015-2016 Markus Teich <markus.teich@stusta.mhn.de>
© 2014 Dimitris Papastamos <sin@2f30.org>
© 2006-2014 Anselm R Garbe <anselm@garbe.us>
© 2014-2016 Laslo Hunhold <dev@frign.de>
© 2016-2023 Hiltjo Posthuma <hiltjo@codemadness.org>

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#include "lib.h"
#include "libexec.h"
#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrandr.h>
#include <X11/extensions/dpms.h>
#include <X11/keysym.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <linux/oom.h>
#include <pwd.h>
#include <shadow.h>
#include <spawn.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

typedef enum { INIT, INPUT, FAILED, NUMCOLS } colour_state;

struct lock {
        int screen;
        Window root, win;
        Pixmap pmap;
        unsigned long colours[NUMCOLS];
        XftDraw *draw;
        XftFont *font;
        XftColor fg;
        XftColor bg;
};

struct xrandr {
        int active;
        int evbase;
        int errbase;
};

static char **ascii_lines;
static size_t ascii_nlines;

//////////////// CONFIG

static const char *colourname[NUMCOLS] = {
    [INIT] = "#000000",   /* after initialization */
    [INPUT] = "#000000",  /* during input */
    [FAILED] = "#000000", /* wrong password */
};

const char *prefix = "                                                                             "
                     "                  ";
const int padding = 40;

const char *const ASCII_FILE = "/del/.dot/suckless/sinit/ascii.current";

static const int failonclear = 1; /* treat a cleared input like a wrong password (colour) */

//////////////// HELPERS

static void readascii(void) {
        FILE *f;
        char *line = NULL;
        size_t size = 0;
        ssize_t len;

        for (int i = 0; i < padding; i++) {
                ascii_lines = realloc(ascii_lines, (ascii_nlines + 1) * sizeof(*ascii_lines));
                if (!ascii_lines) epanic("slock: realloc\n");
                ascii_lines[ascii_nlines++] = strdup("");
        }

        f = fopen(ASCII_FILE, "r");
        if (!f) epanic("slock: fopen ascii");

        while ((len = getline(&line, &size, f)) != -1) {
                if (len > 0 && line[len - 1] == '\n') line[--len] = '\0';

                ascii_lines = realloc(ascii_lines, (ascii_nlines + 1) * sizeof(*ascii_lines));

                if (!ascii_lines) epanic("slock: realloc");

                size_t prefix_len = strlen(prefix);
                ascii_lines[ascii_nlines] = malloc(prefix_len + (size_t)len + 1);
                if (!ascii_lines[ascii_nlines]) epanic("slock: malloc");
                memcpy(ascii_lines[ascii_nlines], prefix, prefix_len);
                memcpy(ascii_lines[ascii_nlines] + prefix_len, line, (size_t)len + 1);
                ascii_nlines++;

                if (!ascii_lines[ascii_nlines - 1]) epanic("slock: strdup");
        }

        free(line);
        fclose(f);
}

static void drawscreen(Display *dpy, struct lock *lock) {
        size_t i;
        int y = 100;
        int lineheight = lock->font->ascent + lock->font->descent;
        XClearWindow(dpy, lock->win);
        for (i = 0; i < ascii_nlines; i++) {
                XftDrawStringUtf8(lock->draw,
                                  &lock->fg,
                                  lock->font,
                                  100,
                                  y,
                                  (const FcChar8 *)ascii_lines[i],
                                  (int)strlen(ascii_lines[i]));

                y += lineheight;
        }
}

static void dontkillme(void) {
        FILE *f;
        const char oomfile[] = "/proc/self/oom_score_adj";
        if (!(f = fopen(oomfile, "w"))) {
                if (errno == ENOENT) return;
                epanic("slock: fopen %s", oomfile);
        }
        fprintf(f, "%d", OOM_SCORE_ADJ_MIN);
        if (fclose(f)) {
                if (errno == EACCES) {
                        epanic("slock: unable to disable OOM killer. "
                               "Make sure to suid or sgid slock.");
                } else {
                        epanic("slock: fclose %s", oomfile);
                }
        }
}

static const char *gethash(void) {
        const char *hash;
        struct passwd *pw;
        errno = 0;
        if (!(pw = getpwuid(getuid()))) epanic("slock: cannot retrieve password entry");
        hash = pw->pw_passwd;
        if (!strcmp(hash, "x")) {
                struct spwd *sp;
                if (!(sp = getspnam(pw->pw_name)))
                        upanic("slock: getspnam: cannot retrieve shadow entry. "
                               "Make sure to suid or sgid slock.");
                hash = sp->sp_pwdp;
        }
        return hash;
}

///////////////// X

static void
readpw(Display *dpy, struct xrandr *rr, struct lock **locks, int nscreens, const char *hash) {
        XRRScreenChangeNotifyEvent *rre;
        char buf[32], passwd[256], *inputhash;
        size_t num;
        int screen, running, failure;
        unsigned int len;
        colour_state colour, oldc;
        KeySym ksym;
        XEvent ev;

        len = 0;
        running = 1;
        failure = 0;
        oldc = INIT;

        while (running && !XNextEvent(dpy, &ev)) {
                if (ev.type == KeyPress) {
                        explicit_bzero(&buf, sizeof(buf));
                        num = (size_t)XLookupString(&ev.xkey, buf, sizeof(buf), &ksym, 0);
                        if (IsKeypadKey(ksym)) {
                                if (ksym == XK_KP_Enter)
                                        ksym = XK_Return;
                                else if (ksym >= XK_KP_0 && ksym <= XK_KP_9)
                                        ksym = (ksym - XK_KP_0) + XK_0;
                        }
                        if (IsFunctionKey(ksym) || IsKeypadKey(ksym) || IsMiscFunctionKey(ksym)
                            || IsPFKey(ksym) || IsPrivateKeypadKey(ksym))
                                continue;
                        switch (ksym) {
                        case XK_Return:
                                passwd[len] = '\0';
                                errno = 0;
                                if (!(inputhash = crypt(passwd, hash)))
                                        fprintf(stderr, "slock: crypt: %s\n", strerror(errno));
                                else
                                        running = !!strcmp(inputhash, hash);
                                if (running) {
                                        XBell(dpy, 100);
                                        failure = 1;
                                }
                                explicit_bzero(&passwd, sizeof(passwd));
                                len = 0;
                                break;
                        case XK_Escape:
                                explicit_bzero(&passwd, sizeof(passwd));
                                len = 0;
                                break;
                        case XK_BackSpace:
                                if (len) passwd[--len] = '\0';
                                break;
                        default:
                                if (num && !iscntrl((int)buf[0]) && (len + num < sizeof(passwd))) {
                                        memcpy(passwd + len, buf, num);
                                        len += (unsigned)num;
                                }
                                break;
                        }
                        colour = len ? INPUT : ((failure || failonclear) ? FAILED : INIT);
                        if (running && oldc != colour) {
                                for (screen = 0; screen < nscreens; screen++)
                                        drawscreen(dpy, locks[screen]);

                                oldc = colour;
                        }
                } else if (rr->active && ev.type == rr->evbase + RRScreenChangeNotify) {
                        rre = (XRRScreenChangeNotifyEvent *)&ev;
                        for (screen = 0; screen < nscreens; screen++) {
                                if (locks[screen]->win == rre->window) {
                                        if (rre->rotation == RR_Rotate_90
                                            || rre->rotation == RR_Rotate_270)
                                                XResizeWindow(dpy,
                                                              locks[screen]->win,
                                                              (unsigned)rre->height,
                                                              (unsigned)rre->width);
                                        else
                                                XResizeWindow(dpy,
                                                              locks[screen]->win,
                                                              (unsigned)rre->width,
                                                              (unsigned)rre->height);
                                        XClearWindow(dpy, locks[screen]->win);
                                        break;
                                }
                        }
                } else if (ev.type == Expose) {
                        for (screen = 0; screen < nscreens; screen++)
                                drawscreen(dpy, locks[screen]);
                } else {
                        for (screen = 0; screen < nscreens; screen++)
                                XRaiseWindow(dpy, locks[screen]->win);
                }
        }
}

static struct lock *lockscreen(Display *dpy, struct xrandr *rr, int screen) {
        char curs[] = {0, 0, 0, 0, 0, 0, 0, 0};
        int i, ptgrab, kbgrab;
        struct lock *lock;
        XColor colour, dummy;
        XSetWindowAttributes wa;
        Cursor invisible;

        if (dpy == NULL || screen < 0 || !(lock = malloc(sizeof(struct lock)))) return NULL;

        lock->screen = screen;
        lock->root = RootWindow(dpy, lock->screen);

        for (i = 0; i < NUMCOLS; i++) {
                XAllocNamedColor(dpy,
                                 DefaultColormap(dpy, lock->screen),
                                 colourname[i],
                                 &colour,
                                 &dummy);
                lock->colours[i] = colour.pixel;
        }

        /* init */
        wa.override_redirect = 1;
        wa.background_pixel = lock->colours[INIT];
        int width = DisplayWidth(dpy, lock->screen);
        int height = DisplayHeight(dpy, lock->screen);
        lock->win = XCreateWindow(dpy,
                                  lock->root,
                                  0,
                                  0,
                                  (unsigned)width,
                                  (unsigned)height,
                                  0,
                                  DefaultDepth(dpy, lock->screen),
                                  CopyFromParent,
                                  DefaultVisual(dpy, lock->screen),
                                  CWOverrideRedirect | CWBackPixel,
                                  &wa);

        lock->draw = XftDrawCreate(dpy,
                                   lock->win,
                                   DefaultVisual(dpy, lock->screen),
                                   DefaultColormap(dpy, lock->screen));

        XSelectInput(dpy, lock->win, ExposureMask);

        if (!lock->draw) upanic("slock: XftDrawCreate failed");

        lock->font = XftFontOpenName(dpy, lock->screen, "monospace:size=10:weight=bold");

        if (!lock->font) upanic("slock: XftFontOpenName failed");

        if (!XftColorAllocName(dpy,
                               DefaultVisual(dpy, lock->screen),
                               DefaultColormap(dpy, lock->screen),
                               "#ff8800",
                               &lock->fg))
                upanic("slock: XftColorAllocName failed");

        lock->pmap = XCreateBitmapFromData(dpy, lock->win, curs, 8, 8);
        invisible = XCreatePixmapCursor(dpy, lock->pmap, lock->pmap, &colour, &colour, 0, 0);
        XDefineCursor(dpy, lock->win, invisible);

        /* Try to grab mouse pointer *and* keyboard for 600ms, else fail the
         * lock */
        for (i = 0, ptgrab = kbgrab = -1; i < 6; i++) {
                if (ptgrab != GrabSuccess) {
                        ptgrab
                            = XGrabPointer(dpy,
                                           lock->root,
                                           False,
                                           ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                                           GrabModeAsync,
                                           GrabModeAsync,
                                           None,
                                           invisible,
                                           CurrentTime);
                }
                if (kbgrab != GrabSuccess) {
                        kbgrab = XGrabKeyboard(dpy,
                                               lock->root,
                                               True,
                                               GrabModeAsync,
                                               GrabModeAsync,
                                               CurrentTime);
                }

                /* input is grabbed: we can lock the screen */
                if (ptgrab == GrabSuccess && kbgrab == GrabSuccess) {
                        XMapRaised(dpy, lock->win);
                        XSetScreenSaver(dpy, 0, 0, DontPreferBlanking, AllowExposures);
                        DPMSDisable(dpy);
                        drawscreen(dpy, lock);
                        if (rr->active) XRRSelectInput(dpy, lock->win, RRScreenChangeNotifyMask);

                        XSelectInput(dpy, lock->root, SubstructureNotifyMask);
                        return lock;
                }

                /* retry on AlreadyGrabbed but fail on other errors */
                if ((ptgrab != AlreadyGrabbed && ptgrab != GrabSuccess)
                    || (kbgrab != AlreadyGrabbed && kbgrab != GrabSuccess))
                        break;

                usleep(100000);
        }

        /* we couldn't grab all input: fail out */
        if (ptgrab != GrabSuccess)
                fprintf(stderr, "slock: unable to grab mouse pointer for screen %d\n", screen);
        if (kbgrab != GrabSuccess)
                fprintf(stderr, "slock: unable to grab keyboard for screen %d\n", screen);
        return NULL;
}

int main(const int argc, Args argv) {
        if (argc > 3 || (argc == 2 && strcmp(argv[1], "sleep"))) upanic("usage: slock [sleep]");
        const bool sleep = argc == 2;

        struct xrandr rr;
        struct lock **locks;
        const char *hash;
        Display *dpy;
        int s, nlocks, nscreens;

        readascii();
        set_lht_level("400", BRIGHTNESS_SET);

        dontkillme();

        hash = gethash();
        errno = 0;
        if (!crypt("", hash)) epanic("slock: crypt");

        if (!(dpy = XOpenDisplay(NULL))) epanic("slock: cannot open display");

        /* check for Xrandr support */
        rr.active = XRRQueryExtension(dpy, &rr.evbase, &rr.errbase);

        /* get number of screens in display "dpy" and blank them */
        nscreens = ScreenCount(dpy);
        if (!(locks = calloc((size_t)nscreens, sizeof(struct lock *))))
                epanic("slock: out of memory");
        for (nlocks = 0, s = 0; s < nscreens; s++) {
                if ((locks[s] = lockscreen(dpy, &rr, s)) != NULL)
                        nlocks++;
                else
                        break;
        }
        XSync(dpy, 0);

        if (nlocks != nscreens) upanic("failed to lock some screens");

        if (sleep) forked_exldn("sudo", "zzz");

        /* everything is now blank. Wait for the correct password */
        readpw(dpy, &rr, locks, nscreens, hash);
        set_lht_level("200", BRIGHTNESS_SET);

        return 0;
}
