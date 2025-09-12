#include "lib.h"
#include "libcmd.h"
#include <assert.h>

const Cmd RG = cmd("g",
                   "rg",
                   "--no-ignore",
                   "=.--hidden",
                   "=N--no-line-number",
                   "=M--max-columns",
                   "=U--multiline",
                   "--ignore-case",
                   "=v--invert-match",
                   "--max-count",
                   "=F--fixed-strings",
                   "=s--case-sensitive",
                   "=S--smart-case",
                   "--count",
                   "=I--no-filename",
                   "-w",
                   "=n--line-number",
                   "--only-matching",
                   "-e",
                   "--quiet",
                   "--pretty",
                   "-u",
                   "=A--after-context",
                   "=B--before-context",
                   "--type",
                   "--binary",
                   "=H--with-filename", );

int main(const int argc, Args argv) {
        run_cli_single((size_t)argc, argv, &RG);
}
