#include "lib.h"
#include "libexec.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const_str SINK = "@DEFAULT_SINK@";
const_str MUTED = "Mute: yes";
const_str UNMUTED = "Mute: no";

static int get_vol_level(void) {
        read_simple_exldn(256, buffer, "pactl", "get-sink-volume", SINK);

        char *space = buffer;
        int i;

        for (i = 0; i < 4; ++i) {
                space = strchr(space, ' ');
                if (space == NULL) upanic("Invalid pactl output after %d", i);
                while (*space == ' ') ++space;
        };

        const int level = atoi(space);
        free(buffer);
        return level;
}

static bool is_mute(void) {
        read_simple_exldn(512, buffer, "pactl", "get-sink-mute", SINK);

        if (!strncmp(buffer, MUTED, strlen(MUTED))) {
                free(buffer);
                return true;
        };

        if (!strncmp(buffer, UNMUTED, strlen(UNMUTED))) {
                free(buffer);
                return false;
        };

        upanic("Invalid mute output: %s", buffer);
}

static const_var_str get_emoji(const bool muted) {
        if (muted)
                return "🔇";
        else
                return "🔊";
}

static const_var_str get_colour(const bool muted) {
        if (muted)
                return "\033[31m";
        else
                return "\033[32m";
}

static void exl_display(void) {
        const bool muted = is_mute();
        const int level = get_vol_level();
        const_str emoji = get_emoji(muted);
        exl_corenotif("%s%d", emoji, level);
}

static void print_display(void) {
        const bool muted = is_mute();
        const int level = get_vol_level();
        const_str emoji = get_emoji(muted);
        const_str colour = get_colour(muted);
        printf("%s%s%d\033[0m\n", colour, emoji, level);
}

static char *unwrap_or(const char *amount, const bool up) {
        if (!amount) { amount = "10"; }
        char *full_diff = malloc(sizeof(char) * 16);
        sprintf(full_diff, "%s%s%%", up ? "+" : "-", amount);
        return full_diff;
}

static void forked_pactl(const_str cmd, const_str action) {
        forked_exldn("pactl", cmd, SINK, action);
}

static void edit_volume(Args argv) {
        if (!strcmp(argv[1], "mute")) {
                forked_pactl("set-sink-mute", "toggle");
                return;
        }

        const bool up = !strcmp(argv[1], "up");

        if (!up && strcmp(argv[1], "down")) exl_err_notif();

        char *const diff = unwrap_or(argv[2], up);
        forked_pactl("set-sink-volume", diff);
        free(diff);
}

int main(const int argc, Args argv) {
        if (argc == 1) {
                print_display();
                return 0;
        }

        edit_volume(argv);
        exl_display();
}
