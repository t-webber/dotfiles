#include "libcmd.h"

make_single("x",
            "xargs",
            "-P",
            "-n",
            "sh",
            "-c",
            "-L",
            "=.-I.",
            "-I",
            "=b-I{}",
            "=w-0", )
