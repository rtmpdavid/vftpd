#ifndef DIR_UTIL_H
#define DIR_UTIL_H

#include <unistd.h>

int file_abs_name(char *abs_name, const char* rel_name, const char *srv_dir, const char* sess_dir);

size_t file_size(const char *abs_name);

int file_stat_string(char* stat, const char* dirname, const char* filename);
int file_exists(const char *abs_name);
int file_is_dir(const char *abs_name);

#endif
