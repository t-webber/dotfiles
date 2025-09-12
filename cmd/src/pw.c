#include "lib.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const_str get_emoji(const bool emoji) {
        char *const status = get_battery_status();
        if (!strcmp(status, "Full")) {
                free(status);
                return emoji ? "💥" : "\033[35m";
        };
        if (!strcmp(status, "Discharging")) {
                free(status);
                return emoji ? "🪫" : "\033[31m";
        };
        if (!strcmp(status, "Charging")) {
                free(status);
                return emoji ? "🔋" : "\033[32m";
        };
        return emoji ? "🤔" : "\033[33m";
}

int main(const int argc, Args argv) {
        const_str emoji = get_emoji(true);
        const_str colour = get_emoji(false);
        const_str level = get_battery_level();
        if (argc == 1) {
                printf("%s%s%s\033[0m\n", colour, emoji, level);
        } else if (!strcmp(argv[1], "notif")) {
                corenotif("%s%s", emoji, level);
        } else {
                panic("Invalid argument")
        }
}
