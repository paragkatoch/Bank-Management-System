// config.c
//============================================================================

// This file contains global vars for client and server

//============================================================================

#include "config.h"

int clientfd;
int serverfd;

User logged_in_user = {0};
int logged_in_user_index = -1;