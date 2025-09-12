#include "lib.h"
#include "libexec.h"
#include <string.h>

const_str EMOJI = "☀️";

int main(int argc, Args argv) {
        if (argc == 2 && !strcmp(argv[1], "up")) {
                forked_exl("/bin/brightnessctl", "brightnessctl", "s", "10+");
        }
        if (argc == 2 && !strcmp(argv[1], "down")) {
                forked_exl("/bin/brightnessctl", "brightnessctl", "s", "10-");
        }

        read_simple_exl(5, "/bin/brightnessctl", "brightnessctl", "g");
        buffer[strlen(buffer) - 1] = '\0';

        printf("\033[35m%s%s%%\033[m\n", EMOJI, buffer);
        corenotif("%s%s%%", EMOJI, buffer);
}
