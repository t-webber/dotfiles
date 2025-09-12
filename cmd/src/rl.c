#include "libcmd.h"

make_single("rl",
            "curl",
            "-L",
            "-O",
            "-i",
            "--progress-bar",
            "-X",
            "-H",
            "GET",
            "POST", )
