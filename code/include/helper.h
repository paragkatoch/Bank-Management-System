#include <stddef.h>

#ifndef HELPER_H
#define HELPER_H

void clear_terminal(int fd);
void safe_strncpy(char *dest, const char *src, size_t n);

#endif
