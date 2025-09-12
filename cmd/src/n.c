#include "lib.h"
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

static void touch_checked(const_str filename) {
        fclose(fopen_checked(filename, "a"));
}

static void mkdir_checked(const_str foldername) {
        if (fopen(foldername, "r") != NULL) {
                epanic("Could not create folder: file "
                       "exists with same name")
        }

        if (mkdir(foldername, 0755)) {
                epanic("Failed to create folder %s", foldername);
        }
}

static void create_file(char *const filename, bool are_all_dir) {
        if (*filename == '\0') { return; }

        char *reader = filename;
        while (*reader != '\0' && *reader != '/') ++reader;

        if (*reader == '\0') {
                if (are_all_dir) {
                        mkdir_checked(filename);
                } else {
                        touch_checked(filename);
                }
                return;
        }

        *reader = '\0';
        mkdir(filename, 0700);

        chdir(filename);
        ++reader;
        create_file(reader, are_all_dir);
}

int main(int argc, char *const *argv) {
        bool are_all_dir = is_verbose(argv[0], "n", "nd");
        if (argc == 1) upanic("Missing arguments...");

        for (int i = 1; i < argc; ++i) {
                if (*argv[i] == '/') chdir("/");
                create_file(argv[i], are_all_dir);
        }
        return 0;
}
