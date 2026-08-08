#include "libcmd.h"

make_single("rl",
            "curl",
            "-L",
            "-O",
            "-o",
            "-i",
            "-v",
            "--progress-bar",
            "-X",
            "-H",
            "GET",
            "POST",
            "-f",
            "-s",
            "-S",
            "=jContent-Type: application/json", )
