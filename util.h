#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <errno.h>
#include <string.h>
#define print_err(title) printf("%s : %s\n", title, strerror(errno))

#endif
