#include "lib.h"
#include <string.h>

int main(const int argc, Args argv) {
        if (argc == 2 && strlen(argv[1]) > 0) {
                int x = atoi(argv[1]);
                if (x != 0) {
                        printf("%c\n", x);
                        return 0;
                }
        }

        upanic("Invalid input");
}
