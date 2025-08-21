#include "lib.h"
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

String get_env_subpath(const String subpath, const_str var) {
	const_str cmd_path = getenv(var);

	const size_t cmd_path_len = strlen(cmd_path);
	const size_t fullpath_len = cmd_path_len + subpath.length;

	str fullpath = malloc(sizeof(char) * fullpath_len);
	char *end = stpcpy(fullpath, cmd_path);
	strcpy(end, subpath.value);

	const String fullpath_str = {.value = fullpath, .length = fullpath_len};
	return fullpath_str;
}

bool is_verbose(const_str program_name,
		const_str normal_name,
		const_str verbose_name) {
	if (!strcmp(program_name, normal_name))
		return false;
	else if (!strcmp(program_name, verbose_name))
		return true;
	else
		panic("Invalid executable name. Please name it `%s` or `%s`.\n",
		      normal_name,
		      verbose_name);
}

const char *argv_one_filename(const int argc, const_str *const argv) {

	if (argc == 1)
		return ".";
	else if (argc == 2)
		return argv[1];
	else
		panic("Too many arguments. Usage: %s [<filename>].\n", argv[0]);
}

bool has_slash(const_str arg, size_t *len, const char **const position) {
	const char *end = arg;
	bool res = false;
	for (; *end != '\0'; ++end)
		if (*end == '/') {
			res = true;
			*position = (end + 1);
		}
	*len = (size_t)(end - arg);
	return res;
}

void store_usage(const_str *const argv) {
	const_str logs = getenv("LOGS");
	const size_t logs_len = strlen(logs);
	str path
	    = malloc(sizeof(char) * (logs_len + sizeof("/aliases.log") + 1));
	char *end = stpcpy(path, logs);
	stpcpy(end, "/aliases.log");
	FILE *fd = fopen(path, "a");
	if (fd == NULL)
		panic("Failed to open file to store telemetry.\n");
	fprintf(fd, "%s\n", argv[0]);
	fclose(fd);
}

size_t utf8_strlen(const char *s) {
	size_t len = 0;
	while (*s) {
		if ((*s & 0xC0) != 0x80)
			len++;
		s++;
	}
	return len;
}
