#include "lib.h"
#include "libcmd.h"
#include <assert.h>

const Cmd FD = cmd("bs",
                   "bsub",
                   "-I",
                   "=8-R 'select[rhe8]'",
                   "-n",
                   "-o",
                   "-W",
                   "=h-W 01:00",
                   "=H-W 04:00",
                   "=d-W 08:00",
                   "=D-W 99:00", );

int main(const int argc, Args argv) {
        run_cli_single((size_t)argc, argv, &FD);
}
