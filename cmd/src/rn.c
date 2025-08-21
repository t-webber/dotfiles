#include "lib.h"
#include "libexec.h"
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static void rm_file(const_str filename) {
	const_str waste = getenv("WASTE");
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	char path[256];
	sprintf(path, "%s/%ld%ld/", waste, ts.tv_sec, ts.tv_nsec);
	printf("%s => %s\n", filename, path);
	mkdir(path, 0755);

	forked_exl("/bin/mkdir", "mkdir", "-p", path);
	exl("/bin/mv", "mv", filename, path);
}

int main(int argc, const_str *const argv) {
	store_usage(argv);
	if (argc == 1)
		panic("Missing arguments...\n");

	for (int i = 1; i < argc; ++i)
		rm_file(argv[i]);

	return 0;
}
