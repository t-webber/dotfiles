#pragma once

#include "lib.h"

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define RESET "\033[0m"
#define ITALIC "\033[3m"
#define NOITALIC "\033[23m"

typedef enum {
        TERMINAL_KITTY,
        TERMINAL_ALACRITTY,
        TERMINAL_OTHER
} terminal_app;

__wur terminal_app what_terminal(void);
void clear(void);

#define OPT(x) YELLOW "[" x NOITALIC YELLOW "]"
#define OPTS " " OPT("..")
#define PROG(x) RED x
#define ARG(x) MAGENTA x
#define PLACE(x) MAGENTA "<" GREEN x MAGENTA ">"
#define S " "
#define U CYAN "Usage:"
