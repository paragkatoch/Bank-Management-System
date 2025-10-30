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

static int __find_account_based_on_accountId_cmp(void *rec, void *ctx)
{
    Account *account = (Account *)rec;
    int userId = *(int *)ctx;

    return account->accountId == userId ? 1 : 0;
}

static int __find_Account_From_AccountId(int fd, int accountId, Account *account)
{
    int index;

    clear_terminal(fd);
    if ((index = record__search(account, sizeof(account), ACCOUNT_DB, &__find_account_based_on_accountId_cmp, &accountId)) == -1)
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

    record__save(&new_account, sizeof(new_account), ACCOUNT_DB);
}

//============================================================================

// View Balance

//============================================================================

void account_view_balance()
{
    int fd = clientfd;
    int accountId = logged_in_user.userId;
    Account account;

    if (__find_Account_From_AccountId(fd, accountId, &account) == -1)
        goto cleanup;

    send_message(fd, makeString("\n\nYour Current Balance is : %d\n", account.accountBalance));
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

    if ((index = __find_Account_From_AccountId(fd, accountId, &account)) == -1)
        goto cleanup;

    // Show current balance
    send_message(fd, makeString("\n\nYour Current balance is : %d\n", account.accountBalance));

    // Ask user for deposit amount

    prompt_user_input(fd, "\nEnter the amount to deposit: ", &temp);
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
    if (record__update(&account, sizeof(account), ACCOUNT_DB, index) == -1 ||
        transaction_save_transaction(accountId, -1, oldBalance, depositAmount, account.accountBalance) == -1)
    {
        send_message(fd, "\nUnable to process transaction");
        waitTillEnter(fd);
        goto cleanup;
    }

    send_message(fd, makeString("\n\nDeposit successful!\nPrevious Balance: %d\nDeposited Amount: %d\nUpdated Balance: %d\n",
                                oldBalance, depositAmount, account.accountBalance));

    waitTillEnter(fd);

cleanup:
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

    if ((index = __find_Account_From_AccountId(fd, accountId, &account)) == -1)
        goto cleanup;

    // Show current balance
    int oldBalance = account.accountBalance;
    send_message(fd, makeString("\n\nYour current balance is : %d\n", oldBalance));

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
    if (record__update(&account, sizeof(account), ACCOUNT_DB, index) == -1 ||
        transaction_save_transaction(accountId, -1, oldBalance, -1 * withdrawAmount, account.accountBalance) == -1)
    {
        send_message(fd, "\nUnable to process transaction");
        goto cleanup;
    }

    // Show confirmation and new balance
    send_message(fd, makeString("\n\nWithdraw successful!\nPrevious Balance: %d\nWithdraw Amount: %d\nUpdated Balance: %d\n",
                                oldBalance, withdrawAmount, account.accountBalance));

    waitTillEnter(fd);

cleanup:
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

    // Search for the sender’s account
    if ((senderIndex = __find_Account_From_AccountId(fd, accountId, &senderAccount)) == -1)
        goto cleanup;

    // Ask for receiver user ID
    prompt_user_input(fd, "\nEnter the account ID of the account to transfer funds to: ", &temp);
    receiverId = atoi(temp);

    // Search for receiver account
    if ((receiverIndex = __find_Account_From_AccountId(fd, receiverId, &receiverAccount)) == -1)
    {
        send_message(fd, "\nCustomer Account not found\n");
        waitTillEnter(fd);
        goto cleanup;
    }

    // check if user account is inactive

    User tempUser;
    int pos = find_user_based_on_userId(&tempUser, receiverId);

    if (pos == -1 || tempUser.account_active == ACCOUNT_INACTIVE)
    {
        send_message(fd, "\nCustomer Account not found\n");
        waitTillEnter(fd);
        goto cleanup;
    }

    // Ask for transfer amount
    prompt_user_input(fd, "\nEnter the amount to transfer: ", &temp);
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
        record__update(&senderAccount, sizeof(senderAccount), ACCOUNT_DB, senderIndex) == -1 ||
        record__update(&receiverAccount, sizeof(receiverAccount), ACCOUNT_DB, receiverIndex) == -1 ||
        transaction_save_transaction(accountId, receiverId, oldSenderBalance, transferAmount, senderAccount.accountBalance) == -1)
    {
        send_message(fd, "\nUnable to process transaction");
        waitTillEnter(fd);
        goto cleanup;
    }

    // Display confirmation
    send_message(fd, makeString(
                         "\n\nTransfer Successful!\n- Transferred Amount: %d\n- Sender (Account ID: %d) \n- Previous Balance: %d \n- New Balance: %d\n",
                         transferAmount,
                         senderAccount.accountId, oldSenderBalance, senderAccount.accountBalance));

    waitTillEnter(fd);

cleanup:
    free(temp);
    return showStartScreen();
}
