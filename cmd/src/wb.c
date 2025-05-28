#include "lib.h"
#include "unistd.h"

int main(int argc, const char *const *const argv) {
        store_usage(argv);

        if (argc == 1)
                return execlp("/usr/bin/brave", "brave", NULL);

        if (argc == 2 && strchr(argv[1], '.') != NULL)
                return execlp("/usr/bin/brave", "brave", argv[1], NULL);

        int start = 1;
        char *const engine = malloc(32 * sizeof(char));
        if (!strcmp(argv[1], "y")) {
                stpcpy(engine, "youtube");
                start = 2;
        } else {
                stpcpy(engine, "google");
        }

        char search[256];
        char *end = stpcpy(search, "https://www.");
        end = stpcpy(end, engine);
        free(engine);
        end = stpcpy(end, ".com/search?q=");

        for (int i = start; i < argc; ++i) {
                end = stpcpy(end, argv[i]);
                *end++ = '+';
        }
        *--end = '\0';

        return execlp("/usr/bin/brave", "brave", search, NULL);
}
