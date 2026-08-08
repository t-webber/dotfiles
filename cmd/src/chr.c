#include "lib.h"

int main(const int argc, Args argv) {
        for (int i = 1; i < argc; ++i) {
                int x = atoi(argv[i]);
                printf("%c ", x);
        }
        printf("\n");
}
