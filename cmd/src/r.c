#include "lib.h"
#include "libexec.h"
#include <assert.h>
#include <dirent.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static void copy_array(size_t *const idx,
		       const char **const exec_args,
		       Args cli_args,
		       const size_t cli_args_len) {
	for (size_t i = 0; i < cli_args_len; ++i)
		exec_args[(*idx)++] = cli_args[i];
}

static const char *single_work_file_runner(const_str file, const_str ext) {
	if (!strcmp(ext, "py"))
		return "python";

	if (!strcmp(ext, "sage"))
		return "sage";

	if (!strcmp(ext, "html"))
		return "brave";

	str line = malloc(sizeof(char *) * 64);
	FILE *fd = fopen(file, "r");
	if (fd == NULL)
		panic("Failed to open %s.\n", file);

	char *buf = fgets(line, 64, fd);
	fclose(fd);
	if (buf == NULL)
		panic("Failed to read first line of %s.\n", file);

	line[strcspn(line, "\n")] = '\0';

	if (!strncmp(line, "#!", 2))
		return line + 2;

	panic("No runner found for %s.", file)
}

static void run_file(const_str file,
		     const_str ext,
		     const_str *const argv,
		     const size_t argv_len,
		     const bool is_release) {

	if (!strcmp(ext, "c") || !strcmp(ext, "cc") || !strcmp(ext, "cpp")
	    || !strcmp(ext, "rs")) {

		if (!strcmp(ext, "rs")) {
			forked_exl("rustc",
				   "rustc",
				   file,
				   "-o",
				   "a.out",
				   is_release ? "-C" : NULL,
				   "opt-level=3");
		} else {
			const_str cmd = strcmp(ext, "c") == 0 ? "cc" : "c++";
			const_str opt = is_release ? "-Ofast" : "-O0";
			const_str flag = is_release ? "-flto" : "-g";
			forked_exl(cmd, cmd, opt, flag, file, "-o", "a.out");
		}

		const size_t len = argv_len + 2;
		const char **const args = malloc(sizeof(char *) * len);
		size_t idx = 0;

		args[idx++] = "./a.out";
		copy_array(&idx, args, argv, argv_len);
		args[idx++] = NULL;
		exv(args);
	}

	if (!strcmp(ext, "go")) {
		const size_t len = argv_len + 4;
		const char **const args = malloc(sizeof(char *) * len);
		size_t idx = 0;

		args[idx++] = "go";
		args[idx++] = "run";
		args[idx++] = file;
		copy_array(&idx, args, argv, argv_len);
		args[idx++] = NULL;
		exv(args);
	}

	const_str prog = single_work_file_runner(file, ext);

	const size_t len = argv_len + 3;
	const char **const args = malloc(sizeof(char *) * len);
	size_t idx = 0;

	args[idx++] = prog;
	args[idx++] = file;
	copy_array(&idx, args, argv, argv_len);
	args[idx++] = NULL;
	exv(args);
}

static int run_folder(const bool is_release) {
	char cwd[128];
	char *buf = getcwd(cwd, 128);
	if (buf == NULL)
		panic("Failed to get current working directory\n");

	DIR *dirp = opendir(cwd);
	struct dirent *entry;
	while ((entry = readdir(dirp))) {
		const_str name = entry->d_name;

		if (!strcmp(name, "Cargo.toml"))
			exl("cargo",
			    "cargo",
			    "run",
			    is_release ? "--release" : NULL);

		if (!strcmp(name, "go.mod"))
			exl("go", "go", "run", ".");

		if (!strcmp(name, "Makefile"))
			panic("Use m");

		if (!strcmp(name, "package.json")) {
			forked_exl("bun", "bun", "i");
			if (is_release) {
				forked_exl("bun", "bun", "run", "build");
				exl("bun", "bun", "run", "start");
			} else {
				exl("bun", "bun", "run", "dev");
			}
		}
	}

	panic("No runner found in the current folder.\n");
}

static const char *as_extension(const_str file) {
	const char *ptr = file;
	for (; *ptr != '\0'; ++ptr)
		if (*ptr == '.')
			return ptr + 1;
	return ptr;
}

int main(const int argc, Args argv) {
	store_usage(argv);

	const bool is_release = is_verbose(argv[0], "r", "rr");

	if (argc == 1)
		return run_folder(is_release);

	const_str file = argv[1];
	const_str ext = as_extension(file);

	run_file(file, ext, argv + 2, (size_t)argc - 2, is_release);
}
