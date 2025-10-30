// transaction.h

//============================================================================

// This file contains logic and UI related to transactions

//============================================================================

#ifndef Transaction_H
#define Transaction_H

// =======================================
// Transaction Record Structure
// =======================================
typedef struct
{
    int transactionId;
    int previousAmount;
    int transactionAmount;
    int lastAmount;
    int from_uid;
    int to_uid;
    // bool transactionStatus;
    // int transaction_time;
} Transaction;

// =======================================
// Transaction method Declarations
// =======================================

// create and save transaction
int transaction_save_transaction(int from_uid, int to_uid, int prev_amount, int amount, int last_amount);

// view Transaction details
void transaction_view_transactions(int loggedUser);

#endif
