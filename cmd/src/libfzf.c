#include "libfzf.h"
#include "lib.h"
#include <ctype.h>
#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ESCAPE 27

__nonnull(()) __wur char *fzf(const char *const *const options, const size_t count) {
        if (count == 0) return NULL;

        SCREEN *s = newterm(NULL, stdout, stdin);
        set_term(s);
        cbreak();
        noecho();
        keypad(stdscr, TRUE);

        char query[128] = "";
        size_t selected = 0;
        char *choice = NULL;

        while (1) {
                erase();
                printw("Search: %s\n", query);

                size_t match_count = 0;
                int active_match_idx = -1;

                for (size_t i = 0; i < count; i++) {
                        if (strcasestr(options[i], query)) {
                                if (match_count == selected) {
                                        attron(A_REVERSE);
                                        printw("> %s\n", options[i]);
                                        attroff(A_REVERSE);
                                        active_match_idx = (int)i;
                                } else {
                                        printw("  %s\n", options[i]);
                                }
                                match_count++;
                        }
                }

                if (match_count == 0) printw("  (No matches)\n");
                if (selected >= match_count && match_count > 0) selected = match_count - 1;

                int ch = getch();
                if (ch == KEY_UP && selected > 0) {
                        selected--;
                } else if (ch == KEY_DOWN && selected + 1 < match_count) {
                        selected++;
                } else if (ch == '\n' || ch == KEY_ENTER) {
                        if (active_match_idx >= 0)
                                choice = unsafe_const_cast(options[active_match_idx]);
                        break;
                } else if (ch == ESCAPE) {
                        break;
                } else if ((ch == KEY_BACKSPACE || ch == 127 || ch == 8) && strlen(query) > 0) {
                        query[strlen(query) - 1] = '\0';
                } else if (isprint(ch) && strlen(query) < sizeof(query) - 1) {
                        strncat(query, (char *)&ch, 1);
                }
        }

        endwin();
        delscreen(s);
        return choice;
}
