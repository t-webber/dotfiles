#include "lib.h"
#include "libexec.h"

int main(void) {
        store_usage("ru", "", false);

        forked_exldn("rustup", "update");
        forked_exldn("rustup", "default", "nightly");
        exldn("cargo", "install-update", "-a", "-g");
}
