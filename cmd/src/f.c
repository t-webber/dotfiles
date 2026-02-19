#include "libcmd.h"

make_single("f",
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
            "=L--follow",
            "=..",
            "-E",
            "=x--exclude", )
