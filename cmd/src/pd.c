#include "lib.h"
#include "libpaths.h"
#include "libvec.h"

int main(const int argc, Args argv) {
        Vec resolutions = find_paths((size_t)(argc - 1), argv + 1);
        for (size_t i = 0; i < resolutions.len; ++i) printf("%s\n", resolutions.data[i]);
        return !resolutions.len;
}
