#include "lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const char *const UPDATE = "/bin/rustup update";
const char *const DEFAULT = "/bin/rustup default nightly";
const char *const INSTALL_UPDATE = "/bin/cargo install-update -a -g";

int main(void) {
        char command[100];
        const char *const cargo_home = getenv("CARGO_HOME");

        char *end = stpcpy(command, cargo_home);

        printf("update\n");
        stpcpy(end, UPDATE);
        printf("Exec: %s\n", command);
        system(command);

        printf("nightly\n");
        stpcpy(end, DEFAULT);
        printf("Exec: %s\n", command);
        system(command);

        printf("install-update\n");
        stpcpy(end, INSTALL_UPDATE);
        printf("Exec: %s\n", command);
        system(command);
}
