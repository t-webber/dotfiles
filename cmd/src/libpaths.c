#include "libpaths.h"
#include "lib.h"
#include "libos.h"
#include "libterm.h"
#include "libvec.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static bool debug;
static int depth = 0;

static void print_depth(void) {
        const int mod = depth % 7;
        const_str colour = mod == 0   ? RED
                           : mod == 1 ? GREEN
                           : mod == 3 ? YELLOW
                           : mod == 4 ? CYAN
                           : mod == 5 ? MAGENTA
                                      : BLUE;
        printf("%s%*s> ", colour, depth * 2, "");
}

#define sp(...)                      \
        {                            \
                print_depth();       \
                printf(__VA_ARGS__); \
                depth += 1;          \
        }

#define ep(...)                      \
        {                            \
                depth -= 1;          \
                print_depth();       \
                printf(__VA_ARGS__); \
        }

#define c(scope, res) \
        case scope:   \
                return res

#define e(scope, var) c(scope, getenv_checked(var))

static const char *__wur get_scope(const char scope) {

        switch (scope) {

                e('a', "APPS");
                e('b', "BLOB");
                e('c', "CMD_SRC");
                e('d', "DEV");
                c('e', "/etc");
                e('f', "FILES");
                e('g', "OCFG");
                e('h', "HOME");
                c('i', "/boot/efi");
                e('k', "DOT");
                e('l', "LOGS");
                e('m', "CMD");

                e('o', "WORK");
                c('p', "/tmp");

                e('s', "SECRET");
                e('t', "DATA");

                c('v', "/var");
                e('w', "WASTE");
                e('x', "XDG_CONFIG_HOME");
                e('y', "STUDY");

                c('.', "main");
                c(':', "master");
                c('%', "dev");

        default:
                return NULL;
        }
}

static void append(Vec *folders, const_str start, const_str end) {
        const size_t len = (size_t)(end - start);
        if (len == 0) return;
        char *const folder = malloc((len + 1) * sizeof(char));
        memcpy(folder, start, len);
        folder[len] = 0;
        push_v(folders, folder);
}

static __wur Vec parse(const size_t len, Args argv) {
        Vec folders = new_v();
        for (size_t i = 0; i < len; ++i) {
                const char *start = argv[i];
                const char *ch = start;
                for (; *ch; ++ch)
                        if (*ch == '/') {
                                append(&folders, start, ch);
                                start = ch + 1;
                        }
                append(&folders, start, ch);
        }
        return folders;
}

static _Noreturn void select_path(const_str path) {
        printf("%s\n", path);
        exit(0);
}

static __wur Vec resolve_one_folder(const_str parent, const_str folder, const bool last);

/* Resolution of the first folder in the path.
 * 1. Exact file.
 * 2. Exact folder.
 * 3. Char alias to global vardir.
 * 4. Folder in cwd containing that string
 * 5. TODO: search with fd in different places. */
static __wur Vec resolve_first_folder(const_str folder, const bool last) {
        if (debug) sp("resolve_first_folder(%s, %d)\n", folder, last);

        Vec resolutions = new_v();
        const size_t folder_len = strlen(folder);

        const_str pwd = getenv_checked("PWD");
        const size_t pwd_len = strlen(pwd);

        String exact = new_s();
        extend_s(&exact, pwd, pwd_len);
        extend_s(&exact, folder, folder_len);
        if (is_file(exact.data) && last) { select_path(exact.data); };
        if (is_dir(exact.data)) {
                push_v(&resolutions, exact.data);
                if (debug) {
                        ep("resolve_first_folder(%s, %d): [%zu] ", folder, last, resolutions.len);
                        for (size_t i = 0; i < resolutions.len; ++i)
                                printf("%s, ", resolutions.data[i]);
                        printf("\n");
                }
                return resolutions;
        };

        const_str scope = get_scope(folder[0]);
        if (scope) {
                const Vec with_scope = resolve_one_folder(scope, folder + 1, last);
                extend_v(&resolutions, with_scope.data, with_scope.len);
        }

        const Vec current_dir = resolve_one_folder(pwd, folder, last);
        extend_v(&resolutions, current_dir.data, current_dir.len);

        if (debug) {
                ep("resolve_first_folder(%s, %d): [%zu] ", folder, last, resolutions.len);
                for (size_t i = 0; i < resolutions.len; ++i) printf("%s, ", resolutions.data[i]);
                printf("\n");
        }
        return resolutions;
}

/* If don't know yet where, do a resolve_first_folder.
 * Else, search current dir for a folder that matches the wanted folder.
 * TODO: including 0 match for subdirectories */
static __wur Vec resolve_one_folder(const_str parent, const_str folder, const bool last) {
#define ret                                                                                       \
        if (debug) {                                                                              \
                ep("resolve_one_folder(NULL, %s, %d): [%zu] ", folder, last, resolutions.len);    \
                for (size_t i = 0; i < resolutions.len; ++i) printf("%s, ", resolutions.data[i]); \
                printf("\n");                                                                     \
        }                                                                                         \
        return resolutions;

        if (debug) sp("resolve_one_folder(%s, %s, %d)\n", parent, folder, last);

        Vec resolutions = new_v();
        if (parent == NULL) {
                resolutions = resolve_first_folder(folder, last);
                ret
        };
        if (!is_dir(parent)) upanic("parent not a folder");
        if (!strcmp(folder, "")) {
                if (!last) upanic("unreachable");
                push_v(&resolutions, parent);
                ret
        }

        int best_match = 1;
        Vec matches_absolute_paths = new_v();
        Vec matches_remainders = new_v();
        const size_t parent_len = strlen(parent);

        DIR *d = opendir_checked(parent);
        struct dirent *e;
        while ((e = readdir(d))) {
                if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, "..")) continue;

                String this = new_s();
                extend_s(&this, parent, parent_len);
                push_s(&this, '/');
                extend_s(&this, e->d_name, strlen(e->d_name));
                push_s(&this, '\0');

                if (!is_dir(this.data)) {
                        free(this.data);
                        continue;
                }

                int match = 0;
                size_t i = 0, j = 0;
                for (; i < strlen(e->d_name) && j < strlen(folder);) {
                        if (e->d_name[i] == folder[j]) {
                                ++match;
                                ++i;
                                ++j;
                        } else {
                                ++i;
                        }
                }

                if (match > best_match) {
                        best_match = match;
                        while (matches_absolute_paths.len)
                                free(unsafe_const_cast(pop_v(&matches_absolute_paths)));
                        while (matches_remainders.len) {
                                const_str non_heap_folder_pointer = pop_v(&matches_remainders);
                                (void)non_heap_folder_pointer;
                        };
                }
                if (match >= best_match) {
                        push_v(&matches_absolute_paths, this.data);
                        push_v(&matches_remainders, folder + j);
                } else
                        free(this.data);
        }

        for (size_t i = 0; i < matches_absolute_paths.len; ++i) {
                if (*matches_remainders.data[i]) {
                        Vec partials = resolve_one_folder(matches_absolute_paths.data[i],
                                                          matches_remainders.data[i],
                                                          last);
                        extend_v(&resolutions, partials.data, partials.len);
                } else {
                        push_v(&resolutions, matches_absolute_paths.data[i]);
                }
        }

        ret
#undef ret
}

static __wur Vec resolve_path(Args encoded_paths,
                              const size_t encoded_paths_len,
                              const_str parent) {
        if (debug) {
                sp("resolve_path(");
                for (size_t i = 0; i < encoded_paths_len; ++i) { printf("%s, ", encoded_paths[i]); }
                printf("%zu, %s)\n", encoded_paths_len, parent);
        }

        Vec resolutions = new_v();

        if (encoded_paths_len == 0) {
                if (is_dir(parent) || is_file(parent)) { push_v(&resolutions, parent); }
                return resolutions;
        }

        const Vec folder_resolutions
            = resolve_one_folder(parent, encoded_paths[0], encoded_paths_len == 1);

        for (size_t i = 0; i < folder_resolutions.len; ++i) {
                Vec these_resolutions = resolve_path(encoded_paths + 1,
                                                     encoded_paths_len - 1,
                                                     folder_resolutions.data[i]);
                extend_v(&resolutions, these_resolutions.data, these_resolutions.len);
        }

        if (debug) {
                ep("resolve_path(");
                for (size_t i = 0; i < encoded_paths_len; ++i) printf("%s, ", encoded_paths[i]);
                printf("%zu, %s): [%zu] ", encoded_paths_len, parent, resolutions.len);
                for (size_t i = 0; i < resolutions.len; ++i) printf("%s, ", resolutions.data[i]);
                printf("\n");
        }
        return resolutions;
}

__wur __nonnull() Vec find_paths(const size_t len, Args segments) {
        debug = getenv("DEBUG") != NULL;
        Vec resolutions = new_v();
        if (len == 1 && (is_file(segments[0]) || is_dir(segments[0]))) {
                push_v(&resolutions, segments[0]);
                return resolutions;
        };
        Vec folders = parse(len, segments);
        return resolve_path(folders.data, folders.len, NULL);
}
