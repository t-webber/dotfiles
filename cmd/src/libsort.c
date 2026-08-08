#include "libsort.h"
#include "lib.h"
#include <assert.h>
#include <stdbool.h>

__wur __attribute_pure__
__nonnull() static bool is_less_than(const char *first, const char *last) {
        while (*first && *last && *first == *last) ++first, ++last;

        return first[0] == '\0' || (last[0] != '\0' && first[0] < last[0]);
}

__nonnull() static void swap(const char **const array,
                             const size_t idx1,
                             const size_t idx2) {
        const_str temp = array[idx1];
        array[idx1] = array[idx2];
        array[idx2] = temp;
}

__nonnull() void sort(const char **const keybinds,
                      const size_t start,
                      const size_t end) {
        if (start + 1 >= end) return;
        const_str pivot = keybinds[start];

        size_t less = start + 1;
        size_t more = end - 1;

        while (less < more)
                if (is_less_than(keybinds[less], pivot)) {
                        ++less;
                } else {
                        swap(keybinds, less, more--);
                }

        assert(more == less);
        size_t pivot_position;
        if (is_less_than(keybinds[less], pivot)) {
                pivot_position = less;
        } else {
                pivot_position = less - 1;
        }

        swap(keybinds, start, pivot_position);
        sort(keybinds, start, pivot_position);
        sort(keybinds, pivot_position + 1, end);
}
