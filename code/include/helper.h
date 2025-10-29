// helper.h
//============================================================================

// This file contains helper/util functions for the system

//============================================================================

#include <stddef.h>

#ifndef HELPER_H
#define HELPER_H

void clear_terminal(int fd);
void safe_strncpy(char *dest, const char *src, size_t n);
void server_error();
char *makeString(const char *fmt, ...);
int generateUniqueUserId();
int generateUniqueLoanId();
int generateUniqueFeedbackId();
int generateUniqueTransactionId();

#endif
