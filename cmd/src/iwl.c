#include "lib.h"
#include "libexec.h"
#include "libvec.h"
#include <string.h>

__nonnull() static void maxed(size_t *store, const size_t new) {
        if (new > *store) *store = new;
}

__nonnull() __wur static size_t get_max_lengths(char *const buffer,
                                              Vec *const lines) {
        size_t max_sha_len = 0;
        size_t max_branch_len = 0;

        char *line_state = NULL;
        const char *line = strtok_r(buffer, "\n", &line_state);

        for (; line; line = strtok_r(NULL, "\n", &line_state)) {
                push(lines, line);

                size_t i = 0;
                while (line[i] != ' ') ++i;
                while (line[i] == ' ') ++i;
                size_t sha_start = i;
                while (line[i] != ' ') ++i;
                maxed(&max_sha_len, i - sha_start);
                while (line[i] == ' ') ++i;
                maxed(&max_branch_len, strlen(line + i));
        }

        return (max_sha_len << 16) + max_branch_len;
}

__nonnull() static void display(const Vec *const lines,
                            const size_t max_sha_len,
                            const size_t max_branch_len) {

        const_str work = getenv_checked("WORK");
        const size_t work_len = strlen(work);

        const_str dev = getenv_checked("DEV");
        const size_t dev_len = strlen(dev);

        for (size_t i = 0; i < lines->len; ++i) {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
                char *ptr = (char *)lines->data[i];
#pragma GCC diagnostic pop

                char *path = ptr;
                while (*ptr != ' ') ++ptr;
                *ptr++ = '\0';
                while (*ptr == ' ') ++ptr;

                const_str sha = ptr;
                while (*ptr != ' ') ++ptr;
                *ptr++ = '\0';
                while (*ptr == ' ') ++ptr;

                const_str branch = ptr;

                if (!strncmp(path, work, work_len)) {
                        char old = *(path + work_len + 1);
                        char *end = stpcpy(path + work_len - 2, "󰃖");
                        *end = old;
                        path = path + work_len - 2;
                } else if (!strncmp(path, dev, dev_len)) {
                        char old = *(path + dev_len + 1);
                        char *end = stpcpy(path + dev_len - 2, "");
                        *end = old;
                        path = path + dev_len - 2;
                }

                printf("\033[31m%-*s\033[0m \033[32m%-*s\033[0m "
                       "\033[33m%s\033[0m\n",
                       (int)max_branch_len,
                       branch,
                       (int)max_sha_len,
                       sha,
                       path);
        }
}

int main(void) {
        read_simple_exldn((1 << 12), buffer, "git", "worktree", "list");
        Vec lines = new_vec();
        const size_t lengths = get_max_lengths(buffer, &lines);
        const size_t max_sha_len = lengths >> 16;
        const size_t max_branch_len = lengths - (max_sha_len << 16);
        display(&lines, max_sha_len, max_branch_len);
}
