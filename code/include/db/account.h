// account.h

//============================================================================

// This file contains logic and UI related to accounts

//============================================================================

#ifndef Account_H
#define Account_H

// =======================================
// Account Record Structure
// =======================================

typedef struct
{
    int accountId;
    int accountBalance;
} Account;

// =======================================
// Account method Declarations
// =======================================

// Create and save Account
void account_create_account(int accountId, int accountBalance);

// View Balance
void account_view_balance();

// Deposit money
void account_deposit();

// Withdraw money
void account_withdraw();

// Transfer funds
void account_transfer_funds();

#endif
