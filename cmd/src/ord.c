#include "lib.h"


int main(const int argc, Args argv) {
        if (argc != 2 || strlen(argv[1]) != 1)
                upanic("Invalid input: given more than 1 char.");

        store_usage(argv[0], "", false);

        printf("%d\n", argv[1][0]);
}
