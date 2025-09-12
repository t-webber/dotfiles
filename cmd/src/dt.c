#include "lib.h"
#include "libexec.h"
#include <assert.h>
#include <ctype.h>
#include <string.h>

int main(const int argc, Args argv) {
        assert(argc == 2 || (argc == 3 && !strcmp(argv[2], "@")));
        bool dbg = argc == 3;

        const_str arg = argv[1];
        const size_t len = strlen(arg);

        char *const expanded = malloc((len + 2) * sizeof(char));
        char *end = expanded;
        *end++ = '+';

        bool last_alnum = false;

        for (const char *ch = arg; *ch; ++ch) {
                bool this_alnum = isalnum(*ch);
                if (this_alnum && !last_alnum) { *end++ = '%'; }
                last_alnum = this_alnum;
                *end++ = *ch;
        }

        if (dbg)
                printf(">>> %s\n", expanded);
        else
                exldn("date", expanded);
}
