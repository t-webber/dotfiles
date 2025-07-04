#include "lib.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static const String ALIASES_FILE_NAME = NEW_STRING("/sh/aliases");

static bool is_valid_file(FILE *file) {
        return file != NULL && ferror(file) == 0 && feof(file) == 0;
}

static FILE *get_aliases_file(void) {
        const char *const CMD = getenv("CMD");
        const size_t len = strlen(CMD) + ALIASES_FILE_NAME.length;
        char *aliases_path = malloc(sizeof(char) * (len + 1));

        FILE *aliases_file = fopen(aliases_path, "a");

        if (!is_valid_file(aliases_file)) {
                fprintf(stderr, "Failed to open file at path %s.",
                        aliases_path);
                exit(2);
        }

        free(aliases_path);
        return aliases_file;
}

int main(const int argc, Argv argv) {
        store_usage(argv);
        if (argc != 3) {
                fprintf(stderr, "You must provide two arguments, found %d.\n",
                        argc - 1);
                return 1;
        }

        FILE *aliases = get_aliases_file();
        fprintf(aliases, "al %s \"%s\"\n", argv[1], argv[2]);
        fclose(aliases);
}
