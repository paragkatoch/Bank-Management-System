// communication.h
//============================================================================

// This file contains logic for communicating between client and server

//============================================================================

#include <stddef.h>

#ifndef COMMUNICATION_H
#define COMMUNICATION_H

int send_message(int fd, const char *msg);
int receive_message(int fd, char **buffer);

#endif
