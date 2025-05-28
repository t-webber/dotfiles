#include "lib.h"
#include <assert.h>
#include <stdio.h>

static char *get_path(void) {
        const char *const config = getenv("XDG_CONFIG_HOME");
        char *path = malloc(sizeof(char) * 128);
        char *end = stpcpy(path, config);
        stpcpy(end, "/sxhkd/sxhkdrc");
        return path;
}

static char **get_keybinds(size_t *const keybinds_index) {
        char **const keybinds = malloc(sizeof(char *) * 64);
        char *current = NULL;
        char *current_end = NULL;
        char buff[128];

        char *path = get_path();
        FILE *fd = fopen(path, "r");
        if (fd == NULL)
                panic("File not found: %s\n", path);
        free(path);

        while (fgets(buff, sizeof(buff), fd)) {

                const size_t len = strlen(buff);
                buff[len - 1] = '\0';

                if (buff[0] == '#' || buff[0] == '\0')
                        continue;

                if (buff[0] >= 'A' && buff[0] <= 'z') {
                        if (current != NULL)
                                keybinds[(*keybinds_index)++] = current;
                        current = malloc(sizeof(char) * 512);
                        current_end = current;
                }

                if (!strncmp(buff, "super", 5)) {
                        current_end = stpcpy(current_end, " ");
                        current_end = stpcpy(current_end, buff + 6);
                } else {
                        current_end = stpcpy(current_end, buff);
                }
        }

        fclose(fd);

        return keybinds;
}

static bool wrapper(const char *const first, const char *const last) {
        return first[0] == '\0' ||
               (last[0] != '\0' &&
                (first[0] < last[0] ||
                 (first[0] == last[0] && wrapper(first + 1, last + 1))));
}

static bool is_less_than(const char *const first, const char *const last) {
        bool res = wrapper(first, last);
        char x;
        if (res) {
                x = '<';
        } else {
                x = '>';
        };
        printf("%.10s\t\033[31m%c\033[0m\t%.10s\n", first, x, last);
        return res;
}

static void swap(char **const array, const size_t idx1, const size_t idx2) {
        char *temp = array[idx1];
        array[idx1] = array[idx2];
        array[idx2] = temp;
}

static void sort(char **const keybinds, const size_t start, const size_t end) {
        printf("\033[32m=== sorting keybinds between %lu and %lu.\n\033[0m",
               start, end);
        if (start + 1 >= end)
                return;
        char *const pivot = malloc(512 * sizeof(char));
        stpcpy(pivot, keybinds[start]);

        size_t less = start + 1;
        size_t more = end - 1;
        while (less <= more) {
                if (is_less_than(keybinds[less], pivot)) {
                        ++less;
                } else {
                        swap(keybinds, less, more--);
                }
        }

        // | < | < | ... | < | < | x | ? | ... | ? | > | ... | > | NULL
        //   ^start                ^less         ^more             ^end

        assert(more + 1 == less);

        // | < | < | ... | < | < | x | > | ... | > | NULL
        //   ^s                    ^l=m            ^end

        //         printf("less was increased to %lu.\n", less);
        //         printf("more was decreased to %lu.\n", more);

        swap(keybinds, less, start);
        sort(keybinds, start, less);
        sort(keybinds, less, end);
}

int main(void) {
        size_t last_index = 0;
        char **const keybinds = get_keybinds(&last_index);
        sort(keybinds, 0, last_index);

        //         for (size_t idx = 0; idx < last_index; ++idx)
        //                 printf("%s\n", keybinds[idx]);
}
