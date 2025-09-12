#include "lib.h"
#include "libcmd.h"
#include <assert.h>

const Cmd FD = cmd("f",
                   "fd",
                   "=I--no-ignore",
                   "=H--hidden",
                   "--absolute-paths",
                   "--type",
                   "=s--case-sensitive",
                   "--ignore-case",
                   "--glob",
                   "=F--fixed-strings",
                   "=d--max-depth",
                   "--extension",
                   "=L--follow", );

int main(const int argc, Args argv) {
        run_cli_single((size_t)argc, argv, &FD);
}
