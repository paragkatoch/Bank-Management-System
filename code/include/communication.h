#include <stddef.h>

#ifndef COMMUNICATION_H
#define COMMUNICATION_H

int send_message(int fd, const char *msg);
int receive_message(int fd, char **buffer);

#endif
