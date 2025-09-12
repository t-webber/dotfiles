#include "lib.h"
#include "libexec.h"
#include <string.h>

static const_str EMOJI = "☀️";

static void exl_notif_level(void) {
        read_simple_exl(5, "/bin/brightnessctl", "brightnessctl", "g");
        char *newline = strchr(buffer, '\n');
        *newline = '\0';
        exl_corenotif("%s%s%%", EMOJI, buffer);
}

static void print_level(void) {
        read_simple_exl(5, "/bin/brightnessctl", "brightnessctl", "g");
        buffer[strlen(buffer) - 1] = '\0';

        printf("\033[35m%s%s%%\033[m\n", EMOJI, buffer);
}

static void set_lht_level(const char *amount, const bool up) {
        if (!amount) amount = "10";
        char *const full = malloc(sizeof(char) * 16);
        sprintf(full, "%s%s", amount, up ? "+" : "-");
        forked_exl("/bin/brightnessctl", "brightnessctl", "s", full);
        free(full);
}

int main(int argc, Args argv) {
        if (argc == 1) {
                print_level();
                return 0;
        }

        const bool up = !strcmp(argv[1], "up");
        if (!up && strcmp(argv[1], "down")) exl_err_notif();

        set_lht_level(argv[2], up);
        exl_notif_level();
}
