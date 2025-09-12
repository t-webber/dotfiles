#include "lib.h"
#include "libexec.h"

static const_var_str get_emoji(const bool emoji) {
        switch (get_battery_status()) {
        case BATTERY_STATUS_CHARGING:
                return emoji ? "🔋" : "\033[32m";
        case BATTERY_STATUS_DISCHARGING:
                return emoji ? "🪫" : "\033[31m";
        case BATTERY_STATUS_FULL:
                return emoji ? "💥" : "\033[35m";
        case BATTERY_STATUS_UNKNOWN:
        default:
                upanic("Failed to get battery status");
        }
}

int main(const int argc, Args argv) {
        store_usage(argv[0], "", false);
        const_str emoji = get_emoji(true);
        const_str colour = get_emoji(false);
        const char *level = get_battery_level();
        if (!level) level = "???";
        if (argc == 1) {
                printf("%s%s\033[0m\n", colour, level);
        } else if (!strcmp(argv[1], "notif")) {
                exl_corenotif("%s%s", emoji, level);
        } else {
                upanic("Invalid argument")
        }
}
