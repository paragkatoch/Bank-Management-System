// account.c

//============================================================================

// This file contains logic and UI related to accounts

//============================================================================

#include "db/account.h"
#include "file_operation.h"
#include "config.h"
#include "db/account.h"
#include "start_screen.h"
#include <string.h>
#include "communication.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "db/transaction.h"
#include "db/user.h"
#include "helper.h"
#include <fcntl.h>

static int __find_account_based_on_accountId_cmp(void *rec, void *ctx)
{
    Account *account = (Account *)rec;
    int userId = *(int *)ctx;

    return account->accountId == userId ? 1 : 0;
}

int find_Account_From_AccountId(int fd, int accountId, Account *account, int lock)
{
    int index;

    clear_terminal(fd);
    if ((index = record__search(account, sizeof(account), ACCOUNT_DB, &__find_account_based_on_accountId_cmp, &accountId, lock)) == -1)
    {
        send_message(fd, "\nAccount not found.\n");
        waitTillEnter(fd);
        return -1;
    }

    return index;
}

//============================================================================

// Create and save Account

//============================================================================

void account_create_account(int accountId, int accountBalance)
{
    Account new_account;
    new_account.accountId = accountId;
    new_account.accountBalance = accountBalance;

    record__save(&new_account, sizeof(new_account), ACCOUNT_DB, RECORD_USE_LOCK);
}

//============================================================================

// View Balance

//============================================================================

void account_view_balance()
{
    int fd = clientfd;
    int accountId = logged_in_user.userId;
    Account account;

    if (find_Account_From_AccountId(fd, accountId, &account, RECORD_USE_LOCK) == -1)
        goto cleanup;

    send_message(fd, "\n╔════════════════════════════════════════════════════════╗\n");
    send_message(fd, "║              ACCOUNT BALANCE ◄                     ║\n");
    send_message(fd, "╚════════════════════════════════════════════════════════╝\n");
    send_message(fd, makeString("\nCurrent Balance: ₹%d\n", account.accountBalance));
    waitTillEnter(fd);

cleanup:
    return showStartScreen();
}

//============================================================================

// Deposit money

//============================================================================

void account_deposit()
{
    char *temp = NULL;
    int fd = clientfd;
    int accountId = logged_in_user.userId;

    int depositAmount = 0;
    int index;
    Account account;
    int lock_fd = -1;

    // get index
    if ((index = find_Account_From_AccountId(fd, accountId, &account, RECORD_USE_LOCK)) == -1)
        goto cleanup;

    // lock record
    ssize_t size = sizeof(Account);
    lock_fd = lock_record_fd(ACCOUNT_DB, F_WRLCK, index * size, size);
    if (lock_fd == -1)
        goto cleanup;

    find_Account_From_AccountId(fd, accountId, &account, RECORD_NOT_USE_LOCK);

    // Show current balance
    send_message(fd, "\n╔════════════════════════════════════════════════════════╗\n");
    send_message(fd, "║                 DEPOSIT MONEY ◄                    ║\n");
    send_message(fd, "╚════════════════════════════════════════════════════════╝\n");
    send_message(fd, makeString("\nCurrent Balance: ₹%d\n", account.accountBalance));

    // Ask user for deposit amount

    prompt_user_input(fd, "\nEnter amount to deposit: ", &temp);
    depositAmount = atoi(temp);

    if (depositAmount <= 0)
    {
        send_message(fd, "\nInvalid amount entered.\n");
        waitTillEnter(fd);
        goto cleanup;
    }

    // Update account balance and save
    int oldBalance = account.accountBalance;
    account.accountBalance += depositAmount;

    // Save updated account back to database and Record the deposit transaction
    if (record__update(&account, size, ACCOUNT_DB, index, RECORD_NOT_USE_LOCK) == -1 ||
        transaction_save_transaction(-1, accountId, oldBalance, depositAmount, account.accountBalance) == -1)
    {
        send_message(fd, "\nUnable to process transaction");
        waitTillEnter(fd);
        goto cleanup;
    }

    send_message(fd, "\nDeposit Successful!\n\n");
    send_message(fd, makeString("  Previous Balance:  ₹%d\n", oldBalance));
    send_message(fd, makeString("  Deposited Amount:  ₹%d\n", depositAmount));
    send_message(fd, makeString("  Updated Balance:   ₹%d\n", account.accountBalance));

    waitTillEnter(fd);

cleanup:
    (lock_fd != -1) && unlock_record(lock_fd, index * size, size);
    free(temp);
    return showStartScreen();
}

//============================================================================

// Withdraw money

//============================================================================

void account_withdraw()
{
    char *temp = NULL;
    int fd = clientfd;
    int accountId = logged_in_user.userId;

    Account account;
    int withdrawAmount = 0;
    int index;
    int lock_fd = -1;

    // get index
    if ((index = find_Account_From_AccountId(fd, accountId, &account, RECORD_USE_LOCK)) == -1)
        goto cleanup;

    // lock record
    ssize_t size = sizeof(Account);
    lock_fd = lock_record_fd(ACCOUNT_DB, F_WRLCK, index * size, size);
    if (lock_fd == -1)
        goto cleanup;

    find_Account_From_AccountId(fd, accountId, &account, RECORD_NOT_USE_LOCK);

    // Show current balance
    int oldBalance = account.accountBalance;
    send_message(fd, "\n╔════════════════════════════════════════════════════════╗\n");
    send_message(fd, "║                WITHDRAW MONEY ◄                    ║\n");
    send_message(fd, "╚════════════════════════════════════════════════════════╝\n");
    send_message(fd, makeString("\nCurrent Balance: ₹%d\n", oldBalance));

    // Get withdraw amount
    prompt_user_input(fd, "\nEnter the amount to withdraw: ", &temp);
    withdrawAmount = atoi(temp);

    if (withdrawAmount <= 0 || withdrawAmount > oldBalance)
    {
        send_message(fd, "\nInvalid amount entered.\n");
        waitTillEnter(fd);
        goto cleanup;
    }

    // Update account balance and save
    account.accountBalance -= withdrawAmount;

    // Save updated account back to database and Record the deposit transaction
    if (record__update(&account, sizeof(account), ACCOUNT_DB, index, RECORD_NOT_USE_LOCK) == -1 ||
        transaction_save_transaction(accountId, -1, oldBalance, -1 * withdrawAmount, account.accountBalance) == -1)
    {
        send_message(fd, "\nUnable to process transaction");
        goto cleanup;
    }

    // Show confirmation and new balance
    send_message(fd, "\nWithdrawal Successful!\n\n");
    send_message(fd, makeString("  Previous Balance:  ₹%d\n", oldBalance));
    send_message(fd, makeString("  Withdrawn Amount:  ₹%d\n", withdrawAmount));
    send_message(fd, makeString("  Updated Balance:   ₹%d\n", account.accountBalance));

    waitTillEnter(fd);

cleanup:
    (lock_fd != -1) && unlock_record(lock_fd, index * size, size);
    free(temp);
    return showStartScreen();
}

//============================================================================

// Transfer funds

//============================================================================

void account_transfer_funds()
{
    char *temp = NULL;
    int fd = clientfd;
    int accountId = logged_in_user.userId; // Sender’s account ID

    Account senderAccount, receiverAccount;
    int senderIndex, receiverIndex;
    int transferAmount = 0;
    int receiverId = 0;
    int lock_fd1 = -1, lock_fd2 = -1;

    // get index
    if ((senderIndex = find_Account_From_AccountId(fd, accountId, &senderAccount, RECORD_USE_LOCK)) == -1)
        goto cleanup;

    // lock record
    ssize_t size = sizeof(Account);
    lock_fd1 = lock_record_fd(ACCOUNT_DB, F_WRLCK, senderIndex * size, size);
    if (lock_fd1 == -1)
        goto cleanup;

    find_Account_From_AccountId(fd, accountId, &senderAccount, RECORD_NOT_USE_LOCK);

    send_message(fd, "\n╔════════════════════════════════════════════════════════╗\n");
    send_message(fd, "║                TRANSFER FUNDS ◄                    ║\n");
    send_message(fd, "╚════════════════════════════════════════════════════════╝\n");
    prompt_user_input(fd, "\nEnter recipient Account ID (-1 to cancel): ", &temp);
    receiverId = atoi(temp);

    // Search for receiver account
    if ((receiverIndex = find_Account_From_AccountId(fd, receiverId, &receiverAccount, RECORD_USE_LOCK)) == -1)
        goto cleanup;

    // lock record
    lock_fd2 = lock_record_fd(ACCOUNT_DB, F_WRLCK, receiverIndex * size, size);
    if (lock_fd2 == -1)
        goto cleanup;

    find_Account_From_AccountId(fd, receiverId, &receiverAccount, RECORD_NOT_USE_LOCK);

    // check if user account is inactive

    User tempUser;
    int pos = find_user_based_on_userId(&tempUser, receiverId, RECORD_USE_LOCK);

    if (pos == -1 || tempUser.account_active == ACCOUNT_INACTIVE)
    {
        send_message(fd, "\nCustomer account not found\n");
        waitTillEnter(fd);
        goto cleanup;
    }

    // Ask for transfer amount
    prompt_user_input(fd, "\nEnter amount to transfer (-1 to cancel): ", &temp);
    transferAmount = atoi(temp);

    if (transferAmount <= 0)
    {
        send_message(fd, "\nInvalid transfer amount.\n");
        waitTillEnter(fd);
        goto cleanup;
    }

    // Check sender’s balance
    if (senderAccount.accountBalance < transferAmount)
    {
        send_message(fd, "\nInsufficient balance.\n");
        waitTillEnter(fd);
        goto cleanup;
    }

    // Perform transfer
    int oldSenderBalance = senderAccount.accountBalance;
    senderAccount.accountBalance -= transferAmount;
    receiverAccount.accountBalance += transferAmount;

    // Update both records in DB
    if (
        record__update(&senderAccount, sizeof(senderAccount), ACCOUNT_DB, senderIndex, RECORD_NOT_USE_LOCK) == -1 ||
        record__update(&receiverAccount, sizeof(receiverAccount), ACCOUNT_DB, receiverIndex, RECORD_NOT_USE_LOCK) == -1 ||
        transaction_save_transaction(accountId, receiverId, oldSenderBalance, transferAmount, senderAccount.accountBalance) == -1)
    {
        send_message(fd, "\nUnable to process transaction");
        waitTillEnter(fd);
        goto cleanup;
    }

    // Display confirmation
    send_message(fd, "\nTransfer Successful!\n\n");
    send_message(fd, makeString("  Transferred Amount:  ₹%d\n", transferAmount));
    send_message(fd, makeString("  Sender Account ID:   %d\n", senderAccount.accountId));
    send_message(fd, makeString("  Previous Balance:    ₹%d\n", oldSenderBalance));
    send_message(fd, makeString("  New Balance:         ₹%d\n", senderAccount.accountBalance));

    waitTillEnter(fd);

cleanup:
    (lock_fd1 != -1) && unlock_record(lock_fd1, senderIndex * size, size);
    (lock_fd2 != -1) && unlock_record(lock_fd2, receiverIndex * size, size);
    free(temp);
    return showStartScreen();
}
