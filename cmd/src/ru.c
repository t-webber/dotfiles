#include "lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const_str UPDATE = "/bin/rustup update";
const_str DEFAULT = "/bin/rustup default nightly";
const_str INSTALL_UPDATE = "/bin/cargo install-update -a -g";

int main(void) {
	char command[100];
	const_str cargo_home = getenv("CARGO_HOME");

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
