/// Adds an alias to the `aliases` file.
///
/// Usage:
/// - First argument: alias's name
/// - Second argument: alias's value
///

#include "lib.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static const String ALIASES_FILE_NAME = NEW_STRING("/sh/aliases");

static bool is_valid_file(FILE *file) {
  return file != NULL && ferror(file) == 0 && feof(file) == 0;
}

static FILE *get_aliases_file() {
  const String aliases_path = get_env_subpath(ALIASES_FILE_NAME, "CMD");
  FILE *aliases_file = fopen(aliases_path.value, "a");

  if (!is_valid_file(aliases_file)) {
    fprintf(stderr, "Failed to open file at path %s.", aliases_path.value);
    exit(2);
  }

  free((void *)aliases_path.value);
  return aliases_file;
}

int main(const int argc, const char *const *const argv) {
  if (argc != 3) {
    fprintf(stderr, "You must provide two arguments, found %d.\n", argc - 1);
    return 1;
  }

  FILE *aliases = get_aliases_file();
  fprintf(aliases, "alias %s=\"%s\"\n", argv[1], argv[2]);
  fclose(aliases);
}
