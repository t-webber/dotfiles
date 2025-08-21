#include "lib.h"
#include "libfiles.h"
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main(const int argc, const_str *const argv) {
	store_usage(argv);
	const char *filename = argv_one_filename(argc, argv);

	struct stat st;
	int x = stat(filename, &st);

	if (x != 0)
		panic("Failed to read %s: exited with code %d.\n",
		      filename,
		      errno);

	if (S_ISDIR(st.st_mode)) {
		return execlp("nvim", "nvim", filename, NULL);
	}

	if (!S_ISREG(st.st_mode))
		panic("Invalid file %s: type %d.", filename, st.st_mode);

	const size_t len = strlen(filename);
	const_str extension = get_filename_extension(filename, len);
	const bool is_not_alacritty = strcmp(getenv("TERM"), "alacritty");
	return exec_open_file(filename,
			      extension,
			      true,
			      is_not_alacritty,
			      false);
}
