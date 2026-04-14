#include "lib.h"
#include "libcmd.h"
#include "libvec.h"

const Cmd COMMANDS[] = {
    cmd("a", "add", ),

    cmd("bi", "binstall", "-y", "--git", ),
    cmd("be", "bench", ),
    cmd("b", "build", "-r", "-p", ),

    cmd("ee", "tree", ),
    cmd("e", "expand", ),

    cmd("f", "fmt", "--all", "--unstable-features", "--error-on-unformatted", ),

    cmd("ii", "init", ),
    cmd("iu", "install-update", "-a", "-g", ),
    cmd("i", "install", "--git", ),

    cmd("lo", "login", ),
    cmd("l", "license", "--direct-deps-only", ),

    cmd("n", "clean", ),

    cmd("o",
        "doc",
        "--all",
        "--open",
        "--document-private-items",
        "--no-deps",
        "-D warnings", ),

    cmd("ph", "publish", ),
    cmd("p", "tarpaulin", "=hHtml", "--doc", "--tests", "--fail-under", "-o", ),

    cmd("rm", "remove", ),
    cmd("ro", "report", "future-incompat", ),
    cmd("r", "run", "-r", "--example", "-p", ),

    cmd("s", "search", "--limit", ),

    cmd("ti", "tauri", "dev", "android", "build", "=oicon", "init", ),
    cmd("t",
        "test",
        "--all",
        "--doc",
        "--jobs",
        "=.___",
        "--release",
        "=n--no-fail-fast",
        "=u--no-run",
        "=c--no-capture", ),

    cmd("ui", "uitest", ),
    cmd("ub", "uibless", ),
    cmd("u", "uninstall", ),

    cmd("v", "dev", ),

    cmd("wa",
        "watch",
        "=x--exec",
        "--shell",
        "--clear",
        "--delay",
        "test",
        "build",
        "=yclippy", ),
    cmd("w", "new", "--lib", ),

    cmd("y",
        "clippy",
        "--all",
        "--examples",
        "--fix",
        "=u--future-incompat-report",
        "=w--allow-dirty",
        "-D warnings",
        "=R-D clippy::restriction", ),

    cmd("z",
        "fuzz",
        "run",
        "tmin",
        "init",
        "=g--target=x86_64-unknown-linux-gnu", ),
};

const Manual MANUAL[] = {
    {'?', "--help"},
    {'C', "--color=always"},
    {'-', "--"},
    {'F', "--features"},
    {'A', "--all-targets"},
};

make_settings(SETTINGS, COMMANDS, MANUAL);

int main(const int argc, Args argv) {
        Vec cmd = new_v();
        push_v(&cmd, "cargo");

        if (!strcmp(argv[0], "c")) {
        } else if (!strcmp(argv[0], "cn")) {
                push_v(&cmd, "+nightly");
        } else if (!strcmp(argv[0], "cs")) {
                push_v(&cmd, "+stable");
        } else {
                upanic("Invalid arg0: %s", argv[0]);
        }

        run_cli((size_t)argc, argv, &SETTINGS, &cmd);
}
