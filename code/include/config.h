// config.h
//============================================================================

// This file contains global vars for client and server

//============================================================================

#include <stddef.h>
#include <db/user.h>

#ifndef GLOBALS_H
#define GLOBALS_H

// db locations
#define USER_DB "db/user.dat"
#define ACCOUNT_DB "db/account.dat"
#define LOAN_DB "db/loan.dat"
#define TRANSACTION_DB "db/transaction.dat"
#define FEEDBACK_DB "db/feedback.dat"

// communication message deliminator
#define MSG_DELIM "/|/"

// shared client socket descriptor
extern int clientfd;

// shared server socket descriptor
extern int serverfd;

// details of logged_in_user
extern User logged_in_user;
extern int logged_in_user_index;

#endif
