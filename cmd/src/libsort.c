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

#ifdef TEST
#define len 100
int main(void) {

        const char *array[100]
            = {"hover", "ivory", "joust", "knack", "lumen", "mirth", "noble",
               "apple", "brisk", "charm", "drake", "evoke", "fable", "grasp",
               "orbit", "pique",

               "quell", "reign", "stoic", "thrum", "untie", "crisp", "droll",
               "ember", "flint", "glide", "haste", "inlet", "quark", "rivet",
               "spurn", "truce", "upend", "verge", "whisk", "vigil", "waltz",
               "xenon", "yield", "zesty", "amber", "blunt", "jaunt", "kneel",
               "latch", "moist", "nudge", "opine", "plume", "elude", "frail",
               "ulcer", "vapid", "wreak", "xylem", "yacht", "loamy", "mango",
               "gloat", "haunt", "inane", "jelly", "kinky", "xerox", "yearn",
               "zonal", "axiom", "blaze", "creak", "daisy", "sling", "taunt",
               "nifty", "omega", "pouch", "quirk", "repel", "neigh", "ounce",
               "prank", "quash", "revel", "scoff", "tidal", "gorge", "hymen",
               "irate", "jaded", "kiosk", "lurid", "mossy", "zebra", "agile",
               "brine", "cower", "demon", "evict", "fluke", "upset", "vigor"};

        sort(array, 0, len);
        for (size_t i = 0; i < len; ++i) printf("x = %s\n", array[i]);
}
#endif
