#include "lib.h"

int main(const int argc, Args argv) {
        for (int i = 1; i < argc; ++i)
                for (const char *ptr = argv[i]; *ptr; ++ptr) printf("%d ", *ptr);

        printf("\n");
}
