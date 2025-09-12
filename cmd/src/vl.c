#include "lib.h"
#include "libexec.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const_str SINK = "@DEFAULT_SINK@";
const_str MUTED = "Mute: yes";
const_str UNMUTED = "Mute: no";

static int get_vol_level(void) {
        read_simple_exl(256,
                        "/bin/pactl",
                        "pactl",
                        "get-sink-volume",
                        SINK,
                        NULL);

        char *space = buffer;
        int i;

        for (i = 0; i < 4; ++i) {
                space = strchr(space, ' ');
                if (space == NULL) panic("Invalid pactl output after %d", i);
                while (*space == ' ') ++space;
        };

        const int level = atoi(space);
        free(buffer);
        return level;
}

static bool is_mute(void) {
        read_simple_exl(512, "/bin/pactl", "pactl", "get-sink-mute", SINK);

        if (!strncmp(buffer, MUTED, strlen(MUTED))) {
                free(buffer);
                return true;
        };

        if (!strncmp(buffer, UNMUTED, strlen(UNMUTED))) {
                free(buffer);
                return false;
        };

        panic("Invalid mute output: %s", buffer);
}

static const_str get_emoji(const bool muted) {
        if (muted)
                return "🔇";
        else
                return "🔊";
}

static const_str get_colour(const bool muted) {
        if (muted)
                return "\033[31m";
        else
                return "\033[32m";
}

static void display(void) {
        const bool muted = is_mute();
        const int level = get_vol_level();
        const_str emoji = get_emoji(muted);
        const_str colour = get_colour(muted);
        printf("%s%s%d\033[0m\n", colour, emoji, level);
        corenotif("%s%d", emoji, level);
}

int main(int argc, Args argv) {
        if (argc == 1) { display(); }

        if (!strcmp(argv[1], "mute")) {
                forked_exl("/bin/pactl",
                           "pactl",
                           "set-sink-mute",
                           SINK,
                           "toggle");
        } else if (!strcmp(argv[1], "up")) {
                forked_exl("/bin/pactl",
                           "pactl",
                           "set-sink-volume",
                           SINK,
                           "+10%");
        } else if (!strcmp(argv[1], "down")) {
                forked_exl("/bin/pactl",
                           "pactl",
                           "set-sink-volume",
                           SINK,
                           "-10%");
        } else {
                notif("error");
        }

        display();
}
