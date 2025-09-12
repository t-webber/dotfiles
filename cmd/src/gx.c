#include "lib.h"
#include "libsort.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static char *get_path(void) {
        const_str config = getenv_checked("XDG_CONFIG_HOME");
        char *path = malloc(sizeof(char) * 128);
        char *end = stpcpy(path, config);
        stpcpy(end, "/sxhkd/sxhkdrc");
        return path;
}

static char *replace_first(char *restrict const buff,
                           const char *restrict const from,
                           const_str to,
                           char *temp) {
        char *sub = strstr(buff, from);
        if (!sub) return NULL;

        char *end = stpcpy(temp, to);
        stpcpy(end, sub + strlen(from));

        return stpcpy(sub, temp);
}

static char *
shorten_and_copy(char *restrict from, char *restrict to, char *restrict temp) {
        while (*from < 'A' || *from > 'z') ++from;

        char *reader = from;
        char *writer = from;
        while (*reader) {
                if (*reader != ' '
                    || (reader != from
                        && ((*(reader - 1) == '+' && *(reader + 1) != '}')
                            || *(reader - 1) == ';')))
                        *writer++ = *reader;
                ++reader;
        }
        *writer = '\0';

        replace_first(from, "super", " ", temp);
        replace_first(from, "mod3", " ", temp);
        replace_first(from, "Escape", "󱊷 ", temp);
        replace_first(from, "alt", " ", temp);
        replace_first(from, "space", "␣ ", temp);
        replace_first(from, "Up", "", temp);
        replace_first(from, "Down", "", temp);
        replace_first(from, "Left", "", temp);
        replace_first(from, "Right", "", temp);
        replace_first(from, "shift", " ", temp);
        replace_first(from, "ctrl", "󰮂 ", temp);

        return stpcpy(to, from);
}

static const char **get_keybinds(size_t *const keybinds_index) {
        const char **const keybinds = malloc(sizeof(char *) * 64);
        char *current = NULL;
        char *current_end = NULL;
        char buff[128];
        char temp[128];

        char *path = get_path();
        FILE *fd = fopen_checked(path, "r");
        if (fd == NULL) panic("File not found: %s", path);
        free(path);

        while (fgets(buff, sizeof(buff), fd)) {
                const size_t len = strlen(buff);
                buff[len - 1] = '\0';

                if (buff[0] == '#' || buff[0] == '\0') continue;

                if (buff[0] >= 'A' && buff[0] <= 'z') {
                        if (current != NULL)
                                keybinds[(*keybinds_index)++] = current;
                        current = malloc(sizeof(char) * 512);

                        current_end = shorten_and_copy(buff, current, temp);

                        const size_t buff_len = utf8_strlen(current);
                        for (size_t i = 0; i < 32 - buff_len; ++i)
                                *current_end++ = ' ';
                } else {
                        current_end = stpcpy(current_end, buff);
                }
        }

        fclose(fd);

        return keybinds;
}

int main(void) {
        size_t last_index = 0;
        const char **const keybinds = get_keybinds(&last_index);
        sort(keybinds, 0, last_index);

        for (size_t i = 0; i < last_index; ++i) printf("%s\n", keybinds[i]);
}
