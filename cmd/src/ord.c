#include "lib.h"
#include <string.h>

int main(const int argc, Args argv) {
        if (argc != 2 || strlen(argv[1]) != 1) upanic("Invalid input");

        printf("%d\n", argv[1][0]);
}
