#include "libcmd.h"

make_single("rl",
            "curl",
            "-L",
            "-O",
            "-i",
            "-v",
            "--progress-bar",
            "-X",
            "-H",
            "GET",
            "POST",
            "-f",
            "-s",
            "-S", )
