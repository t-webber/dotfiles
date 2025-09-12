#include "libos.h"
#include "lib.h"
#include <sys/stat.h>
#include <unistd.h>

__nonnull() void chdir_checked(const_str path) {
        if (chdir(path)) { epanic("Failed to change process cwd to %s", path) };
}

__nonnull() void chmod_checked(const_str path, const mode_t mode) {
        if (chmod(path, mode)) epanic("Failed to run chmod %u %s", mode, path);
}

bool __wur __nonnull() is_dir(const_str path) {
        struct stat st;
        if (stat(path, &st)) { return 0; }
        return S_ISDIR(st.st_mode);
}

__nonnull() __wur FILE *fopen_checked(const_str file_name, const_str mode) {
        FILE *f = fopen(file_name, mode);
        if (f == NULL) epanic("Failed to open file %s", file_name);
        return f;
}

__wur __nonnull() DIR *opendir_checked(const_str dirname) {
        DIR *dirp = opendir(dirname);
        if (dirp == NULL) epanic("Failed to open dir %s", dirname);
        return dirp;
}

__nonnull() void closedir_checked(DIR *dirp) {
        if (closedir(dirp)) epanic("Failed to close dir");
}

__wur __nonnull() FILE *popen_checked(const_str command) {
        FILE *f = popen(command, "r");
        if (f == NULL) epanic("Failed to run %s", command);
        return f;
}

__nonnull() void append_file(const_str path, const_str data) {
        FILE *f = fopen_checked(path, "a");
        fputs(data, f);
        fclose(f);
}

__nonnull() __wur bool is_file(const_str file_name) {
        FILE *f = fopen(file_name, "r");
        if (f) {
                fclose(f);
                return true;
        }
        return false;
}

__nonnull() void touch_checked(const_str filename) {
        fclose(fopen_checked(filename, "a"));
}

__wur __nonnull() bool is_file_binary(const_str path) {
        FILE *f = fopen_checked(path, "rb");
        int c;
        while ((c = fgetc(f)) != EOF) {
                if (c == '\n' || c == '\t' || c == '\r') continue;
                if (c == 127 || c < 32) return true;
        }
        return false;
}
