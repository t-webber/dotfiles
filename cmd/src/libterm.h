#pragma once

#include "lib.h"

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define RESET "\033[0m"
#define C(colour, str) colour str RESET

typedef enum {
        TERMINAL_KITTY,
        TERMINAL_ALACRITTY,
        TERMINAL_OTHER
} terminal_app;

__wur terminal_app what_terminal(void);
void clear(void);
