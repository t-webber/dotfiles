#include "lib.h"
#include "libexec.h"

const_str UPDATE = "/bin/rustup update";
const_str DEFAULT = "/bin/rustup default nightly";
const_str INSTALL_UPDATE = "/bin/cargo install-update -a -g";

int main(void) {
        store_usage("ru", "", false);
        char command[100];
        const_str cargo_home = getenv_checked("CARGO_HOME");

        char *end = stpcpy(command, cargo_home);
        char *bin = stpcpy(end, "/bin/");

        stpcpy(bin, "rustup");
        forked_exldn(command, "update");
        forked_exldn(command, "default", "nightly");

        stpcpy(bin, "cargo");
        exldn(command, "install-update", "-a", "-g");
}
