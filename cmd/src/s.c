#include "lib.h"
#include "libfiles.h"
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

static int
ls(const_str filename, const bool is_verbose, const bool has_multiple) {
	if (!is_verbose)
		return execlp("ls", "ls", "-A", filename, NULL);

	if (has_multiple) {
		struct winsize w;
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
		const size_t cols = w.ws_col - 2 - strlen(filename);
		const size_t nb_equals = cols / 2;

		fputs("\033[35m", stdout);
		for (size_t i = 0; i < nb_equals; ++i)
			putchar('=');
		printf(" %s ", filename);
		for (size_t i = 0; i < nb_equals + (cols % 2); ++i)
			putchar('=');
		puts("\033[0m");
	}

	return execlp("eza",
		      "eza",
		      "-lA",
		      "--icons=always",
		      "--no-time",
		      "--no-user",
		      filename,
		      NULL);
}

static int display_file(const_str filename, const bool is_verbose) {
	const size_t len = strlen(filename);
	const_str extension = get_filename_extension(filename, len);
	const bool is_not_alacrity = strcmp(getenv("TERM"), "alacritty");

	return exec_open_file(filename,
			      extension,
			      false,
			      is_not_alacrity,
			      is_verbose);
}

static int
display(const_str filename, const bool is_verbose, const bool has_multiple) {
	struct stat st;
	int x = stat(filename, &st);

	if (x != 0)
		panic("Failed to read %s: %s.\n", filename, strerror(errno));

	if (S_ISDIR(st.st_mode))
		ls(filename, is_verbose, has_multiple);

	if (!S_ISREG(st.st_mode))
		panic("Invalid file %s: type %d.", filename, st.st_mode);

	return display_file(filename, is_verbose);
}

int main(const int argc, const_str *const argv) {
	store_usage(argv);
	bool verbose = is_verbose(argv[0], "s", "sv");

	if (argc == 1) {
		int res = display(".", verbose, false);
		panic("Failed to read .: exec exited with code %d: %s.\n",
		      res,
		      strerror(errno));
	}

	for (int file_nb = 1; file_nb < argc; ++file_nb) {
		pid_t pid = fork();

		if (pid < 0)
			panic("Failed to fork the %d-th time.\n", file_nb);

		if (pid == 0) {
			int res = display(argv[file_nb], verbose, argc > 2);
			panic("Failed to read %s: exec exited with code %d: "
			      "%s.\n",
			      argv[file_nb],
			      res,
			      strerror(errno));
		}

		int status;
		waitpid(pid, &status, 0);
		if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
			panic("Child process failed with status %d.\n",
			      WEXITSTATUS(status));
	}
}
