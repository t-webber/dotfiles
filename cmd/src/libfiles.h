#pragma once
#include <stdbool.h>
#include <stddef.h>

const char *get_filename_extension(const char *const filename,
                                   const size_t len);

bool has_timg_support(const char *const extension);
bool has_brave_support(const char *const extension);

int exec_open_file(const char *const filename, const char *const extension,
                   const bool is_open, const bool is_kitty,
                   const bool is_verbose);
