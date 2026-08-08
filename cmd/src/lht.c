#include "lib.h"
#include "libexec.h"

static const_str EMOJI = "☀️";

_Noreturn static void exl_notif_level(void) {
        read_simple_exldn(5, buffer, "sudo", "brightnessctl", "g");
        char *newline = strchr(buffer, '\n');
        *newline = '\0';
        exl_corenotif("%s%3d", EMOJI, atoi(buffer));
}

static void print_level(void) {
        read_simple_exldn(5, buffer, "sudo", "brightnessctl", "g");
        buffer[strlen(buffer) - 1] = '\0';
        printf("\033[35m%s%s\033[m\n", EMOJI, buffer);
}

int main(int argc, Args argv) {
        bool quiet = !strcmp(argv[0], "lhtq");
        if (argc == 1) {
                print_level();
                return 0;
        }
        if (argc > 3) exl_corenotif("Too many argument for lht command");

        brightness_function f;
        if (!strcmp(argv[1], "up"))
                f = BRIGHTNESS_UP;
        else if (!strcmp(argv[1], "down"))
                f = BRIGHTNESS_DOWN;
        else if (!strcmp(argv[1], "set"))
                f = BRIGHTNESS_SET;
        else
                exl_corenotif("Invalid lht command %s.", argv[1]);

        set_lht_level(argv[2], f);
        if (!quiet) exl_notif_level();
}
