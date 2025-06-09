#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void try_make_line(const char *const arg, char *const line) {
        char *ptr = line;

        if (*ptr < 'a' || *ptr > 'z')
                return;

        while (*++ptr != '\0') {
                if (*ptr >= 'a' && *ptr <= 'z')
                        continue;

                if (*ptr != ':')
                        return;

                *ptr = '\0';
                if (strcmp(line, arg))
                        return;

                char command[32];
                sprintf(command, "clear && make %s", arg);
                system(command);
                exit(0);
        }
}

static void try_make(const char *const arg) {

        FILE *f = fopen("Makefile", "r");
        if (f == NULL)
                return;

        char line[128];
        while (fgets(line, sizeof(line), f)) {
                try_make_line(arg, line);
        }
}

int main(const int argc, char *const *const argv) {
        if (argc == 1) {
                system("clear && make");
                return 0;
        }

        if (argc > 3) {
                printf("Too many arguments...\n");
                return 0;
        }

        char *arg = argv[1];

        if (strcmp(arg, "t") == 0)
                stpcpy(arg, "test");

        if (strcmp(arg, "b") == 0)
                stpcpy(arg, "build");

        if (strcmp(arg, "r") == 0)
                stpcpy(arg, "run");

        if (strcmp(arg, "c") == 0)
                stpcpy(arg, "clean");

        if (strcmp(arg, "rl") == 0)
                stpcpy(arg, "release");

        try_make(arg);

        char command[32];
        sprintf(command, "man -a %s\n", arg);
        system(command);
}
