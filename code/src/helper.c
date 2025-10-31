// helper.c
//============================================================================

// This file contains helper/util functions for the system

//============================================================================

#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "communication.h"
#include "config.h"
#include "db/user.h"
#include "db/loan.h"
#include "db/transaction.h"
#include "db/feedback.h"
#include "file_operation.h"
#include <stdarg.h>
#include "helper.h"
#include <stdio.h>

// Copy src to dest string and set end deliminator
void safe_strncpy(char *dest, const char *src, size_t n)
{
    if (src)
    {
        strncpy(dest, src, n - 1);
        dest[n - 1] = '\0';
    }
    else
    {
        dest[0] = '\0';
    }
}

// Clear user terminal
void clear_terminal(int fd)
{
    // Send ANSI escape sequence to clear screen and move cursor to top
    send_message(fd, "\033[2J\033[H");
}

// Handle Internal Server error
void server_error()
{
    send_message(clientfd, "\n\n Internal Server Error\n\n");
    send_message(clientfd, "Logging out...");
    sleep(1);
    user_logout();
}

// char *makeString(const char *fmt, ...)
// {
//     static char buffer[1000]; // or malloc for dynamic
//     va_list args;
//     va_start(args, fmt);
//     vsnprintf(buffer, sizeof(buffer), fmt, args);
//     va_end(args);
//     return buffer;
// }

char *makeString(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    // get required size
    int size = vsnprintf(NULL, 0, fmt, args) + 1;
    va_end(args);

    char *buffer = malloc(size);
    if (!buffer)
        return NULL;

    va_start(args, fmt);
    vsnprintf(buffer, size, fmt, args);
    va_end(args);

    return buffer; // caller must free()
}

int generateUniqueUserId()
{
    User tempUser;
    if (record_end_record(&tempUser, sizeof(User), USER_DB, RECORD_USE_LOCK) == -1)
        return 1;

    int maxId = tempUser.userId;
    return maxId + 1;
}

int generateUniqueLoanId()
{
    Loan tempLoan;
    if (record_end_record(&tempLoan, sizeof(Loan), LOAN_DB, RECORD_USE_LOCK) == -1)
        return 1;

    int maxId = tempLoan.loanId;
    return maxId + 1;
}

int generateUniqueFeedbackId()
{
    Feedback tempFeedback;
    if (record_end_record(&tempFeedback, sizeof(Feedback), FEEDBACK_DB, RECORD_USE_LOCK) == -1)
        return 1;

    int maxId = tempFeedback.feedbackId;
    return maxId + 1;
}

int generateUniqueTransactionId()
{
    Transaction tempTransaction;
    if (record_end_record(&tempTransaction, sizeof(Transaction), TRANSACTION_DB, RECORD_USE_LOCK) == -1)
        return 1;

    int maxId = tempTransaction.transactionId;
    return maxId + 1;
}

void waitTillEnter(int fd)
{
    char *tmp = NULL;
    send_message(fd, "\n\n\nPress enter to go back...");
    receive_message(fd, &tmp);
    free(tmp);
}

int prompt_user_input(int fd, const char *message, char **out)
{
    send_message(fd, message);
    if (receive_message(fd, out) < 0)
    {
        send_message(fd, "\nError receiving input.\n");
        sleep(2);
        return -2;
    }

    if (strcmp(*out, "-1") == 0)
    {
        return -1;
    }

    return 0;
}
