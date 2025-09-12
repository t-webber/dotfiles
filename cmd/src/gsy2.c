#include "lib.h"
#include "libexec.h"
#include <assert.h>

int main(const int argc, Args argv) {
        assert(argc == 2);
        clear();
        char regex[512];
        sprintf(regex, "^\\S+\\s+\\S+\\s+\\S*%s", argv[1]);
        exldn("rg",
              "--glob",
              "!archtypes.hpp",
              "--glob",
              "!*.simp.cpp",
              "--hidden",
              "--no-ignore",
              regex,
              argv[2]);
}
