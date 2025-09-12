#include "lib.h"
#include "libexec.h"

typedef struct {
        const_str emoji;
        const_str colour;
} battery_decorations;

static battery_decorations get_battery_decorations(const battery_status stat) {
        switch (stat) {
        case BATTERY_STATUS_CHARGING:
                return (battery_decorations){.emoji = "🔋", .colour = "\033[32m"};
        case BATTERY_STATUS_DISCHARGING:
                return (battery_decorations){.emoji = "🪫", .colour = "\033[31m"};
        case BATTERY_STATUS_FULL:
                return (battery_decorations){.emoji = "💥", .colour = "\033[35m"};
        case BATTERY_STATUS_UNKNOWN:
        default:
                upanic("Failed to get battery status");
        }
}

int main(const int argc, Args argv) {
        store_usage(argv[0], "", false);
        const battery_status stat = get_battery_status();
        const battery_decorations bat = get_battery_decorations(stat);
        const char *level = get_battery_level();
        if (!level) level = "???";
        if (argc == 1) {
                printf("%s%s\033[0m\n", bat.colour, level);
        } else if (!strcmp(argv[1], "notif")) {
                exl_corenotif("%s%s", bat.emoji, level);
        } else if (!strcmp(argv[1], "check")) {
                const int perc = atoi(level);
                if (stat != BATTERY_STATUS_DISCHARGING && perc >= 90) {
                        exl_err_notif("Fully charged");
                } else if (stat == BATTERY_STATUS_DISCHARGING && perc <= 10) {
                        exldn("sudo", "zzz");
                } else if (stat == BATTERY_STATUS_DISCHARGING && perc <= 30) {
                        exl_err_notif("Plug in computer");
                }
        } else {
                upanic("Invalid argument")
        }
}
