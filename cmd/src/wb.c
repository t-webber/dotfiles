#include "lib.h"
#include "libexec.h"
#include <string.h>

int main(int argc, Args argv) {
        store_usage(argv);

        if (argc == 1) exl("/usr/bin/brave", "brave");

        if (argc == 2 && strchr(argv[1], '.') != NULL)
                exl("/usr/bin/brave", "brave", argv[1]);

        int start = 1;
        const char *engine;

        if (!strcmp(argv[1], "y")) {
                engine = "youtube";
                start = 2;
        } else {
                engine = "google";
        }

        char search[256];
        char *end = stpcpy(search, "https://www.");
        end = stpcpy(end, engine);
        end = stpcpy(end, ".com/search?q=");

        for (int i = start; i < argc; ++i) {
                end = stpcpy(end, argv[i]);
                *end++ = '+';
        }
        *--end = '\0';

        exl("/usr/bin/brave", "brave", search);
}
