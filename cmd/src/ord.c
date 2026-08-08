#include "lib.h"

int main(const int argc, Args argv) {
        store_usage(argv[0], "", false);
        for (int i = 1; i < argc; ++i) {
                const_str arg = argv[i];
                for (const char *ptr = arg; *ptr; ++ptr) {
                        printf("%d ", *ptr);
                }
        }
        printf("\n");
}
