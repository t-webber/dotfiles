#include "libterm.h"

__wur terminal_app what_terminal(void) {
        const_str term = getenv_checked("TERM");
        if (!strcmp(term, "alacritty")) { return TERMINAL_ALACRITTY; }
        if (!strcmp(term, "xterm-kitty")) { return TERMINAL_KITTY; }
        return TERMINAL_OTHER;
}

void clear(void) {
        printf("\033c");
        fflush(stdout);
}
