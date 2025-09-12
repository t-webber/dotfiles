#include "lib.h"
#include <assert.h>

int main(const int argc, Args argv) {
        assert(argc == 2);
        store_usage(argv[1], "", true);
}
