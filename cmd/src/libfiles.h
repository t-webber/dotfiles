#pragma once

#include "lib.h"
#include <stdbool.h>
#include <stddef.h>

const char *get_filename_extension(const_str filename, const size_t len);

bool has_timg_support(const_str extension);
bool has_brave_support(const_str extension);

int exec_open_file(const_str filename,
		   const_str extension,
		   const bool is_open,
		   const bool is_kitty,
		   const bool is_verbose);
