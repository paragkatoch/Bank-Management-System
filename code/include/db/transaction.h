#ifndef Transaction_H
#define Transaction_H

// =======================================
// Transaction Record Structure
// =======================================
typedef struct {
    int transactionId;
    int transactionAmount;
    int from_uid;
    int to_uid;
    bool transactionStatus;
    int transaction_time;
} Transaction;

// =======================================
// Transaction method Declarations
// =======================================

// create and save transaction
void transaction_create_transaction();

// view Transaction details
void transaction_view_transaction_details();

// view user transaction
void transaction_view_user_transaction();

// view all transactions
void transaction_view_all_transaction();

// view non review transaction
void transaction_view_non_reviewed_transaction();

// review transaction
void transaction_review_transaction();

#endif
