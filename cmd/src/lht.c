#include "lib.h"
#include "libexec.h"


static const_str EMOJI = "☀️";

_Noreturn static void exl_notif_level(void) {
        read_simple_exldn(5, buffer, "sudo", "brightnessctl", "g");
        char *newline = strchr(buffer, '\n');
        *newline = '\0';
        exl_corenotif("%s%s%%", EMOJI, buffer);
}

static void print_level(void) {
        read_simple_exldn(5, buffer, "sudo", "brightnessctl", "g");
        buffer[strlen(buffer) - 1] = '\0';

        printf("\033[35m%s%s%%\033[m\n", EMOJI, buffer);
}

typedef enum { UP, DOWN, SET } function;

static void set_lht_level(const char *amount, const function f) {
        if (!amount) amount = "10"; // default offset and default brightness
        if (f == SET) {
                forked_exldn("sudo", "brightnessctl", "s", amount);
                return;
        }

        char *const full = malloc(sizeof(char) * 16);
        sprintf(full, "%s%s", amount, f == UP ? "+" : "-");
        forked_exldn("sudo", "brightnessctl", "s", full);
}

int main(int argc, Args argv) {
        store_usage(argv[0], "", false);
        if (argc == 1) {
                print_level();
                return 0;
        }
        if (argc > 3) exl_corenotif("Too many argument for lht command");

        function f;
        if (!strcmp(argv[1], "up"))
                f = UP;
        else if (!strcmp(argv[1], "down"))
                f = DOWN;
        else if (!strcmp(argv[1], "set"))
                f = SET;
        else
                exl_corenotif("Invalid lht command %s.", argv[1]);

        set_lht_level(argv[2], f);
        exl_notif_level();
}
