#include "lib.h"
#include "libexec.h"
#include <assert.h>

int main(const int argc, Args argv) {
        assert(argc == 2);
        char regex[512];
        sprintf(regex, "^\\S+\\s+\\S+\\s+\\S*%s", argv[1]);
        exl("rg", "rg", "--hidden", "--no-ignore", regex, argv[2]);
}
