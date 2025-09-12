#include "libcmd.h"

make_single("bs",
            "bsub",
            "-I",
            "=8-R 'select[rhe8]'",
            "-n",
            "-o",
            "-W",
            "=h-W 01:00",
            "=H-W 04:00",
            "=d-W 08:00",
            "=D-W 99:00", )
