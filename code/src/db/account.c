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
#include "helper.h"

// Create and save Account
void account_create_account(int accountId, int accountBalance)
{
    Account new_account;
    new_account.accountId = accountId;
    new_account.accountBalance = accountBalance;

    record__save(&new_account, sizeof(new_account), ACCOUNT_DB);
}

// View Balance
static int __avb__cmp_userID(void *rec, void *ctx)
{
    Account *account = (Account *)rec;
    int userId = *(int *)ctx;

    return account->accountId == userId ? 1 : 0;
}

void account_view_balance()
{
    int fd = clientfd;
    int userId = logged_in_user.userId;

    Account account;
    char *temp = NULL;

    clear_terminal(fd);
    if (record__search(&account, sizeof(account), ACCOUNT_DB, __avb__cmp_userID, &userId) == -1)
    {
        send_message(fd, "\nAccount not found.\n");
        sleep(2);
        goto cleanup;
    }

    char buffer[256];
    snprintf(buffer, sizeof(buffer), "\n\nYour Principal Balance is : %d\n", account.accountBalance);
    send_message(fd, buffer);

    send_message(fd, "\n\n\nPress enter to continue...");
    receive_message(fd, &temp);
    free(temp);

cleanup:
    return showStartScreen();
}

// Deposit money
void account_deposit()
{
    char *temp = NULL;
    int fd = clientfd;
    int userId = logged_in_user.userId;

    Account account;
    int depositAmount = 0;

    clear_terminal(fd);
    int index = record__search(&account, sizeof(account), ACCOUNT_DB, __avb__cmp_userID, &userId);

    if (index == -1)
    {
        send_message(fd, "\nAccount not found.\n");
        sleep(2);
        goto cleanup;
    }

    // Step 2: Show current balance
    char buffer[256];
    snprintf(buffer, sizeof(buffer),
             "\n\nYour current balance is : %d\n", account.accountBalance);
    send_message(fd, buffer);

    // Step 3: Ask user for deposit amount
    send_message(fd, "\nEnter the amount to deposit: ");
    receive_message(fd, &temp);

    // Step 4: Convert string input to integer
    depositAmount = atoi(temp);

    if (depositAmount <= 0)
    {
        send_message(fd, "\nInvalid amount entered.\n");
        sleep(2);
        goto cleanup;
    }

    // Step 5: Update account balance
    int oldBalance = account.accountBalance;
    account.accountBalance += depositAmount;

    // Step 6: Save updated account back to database
    // Record the deposit transaction
    // TODO: undo if unable to save transaction
    if (record__update(&account, sizeof(account), ACCOUNT_DB, index) == -1 ||
        transaction_save_transaction(userId, -1, oldBalance, depositAmount, account.accountBalance) == -1)
    {
        send_message(fd, "Unable to process transaction");
        goto cleanup;
    }

    // Step 7: Show confirmation and new balance
    snprintf(buffer, sizeof(buffer),
             "\n\nDeposit successful!\nPrevious Balance: %d\nDeposited Amount: %d\nUpdated Balance: %d\n",
             oldBalance, depositAmount, account.accountBalance);
    send_message(fd, buffer);

    send_message(fd, "\n\nPress enter to continue...");
    receive_message(fd, &temp);

cleanup:
    free(temp);
    return showStartScreen();
}

// Withdraw money
void account_withdraw()
{
    char *temp = NULL;
    int fd = clientfd;
    int userId = logged_in_user.userId;

    Account account;
    int withdrawAmount = 0;

    clear_terminal(fd);
    int index = record__search(&account, sizeof(account), ACCOUNT_DB, __avb__cmp_userID, &userId);

    if (index == -1)
    {
        send_message(fd, "\nAccount not found.\n");
        sleep(2);
        goto cleanup;
    }

    // Step 2: Show current balance
    int oldBalance = account.accountBalance;
    char buffer[256];

    snprintf(buffer, sizeof(buffer),
             "\n\nYour current balance is : %d\n", oldBalance);
    send_message(fd, buffer);

    // Step 3: Ask user for withdraw amount
    send_message(fd, "\nEnter the amount to withdraw: ");
    receive_message(fd, &temp);

    // Step 4: Convert string input to integer
    withdrawAmount = atoi(temp);

    if (withdrawAmount <= 0 || withdrawAmount > oldBalance)
    {
        send_message(fd, "\nInvalid amount entered.\n");
        sleep(2);
        goto cleanup;
    }

    // Step 5: Update account balance
    account.accountBalance -= withdrawAmount;

    // Step 6: Save updated account back to database
    // Record the deposit transaction
    // TODO: undo if unable to save transaction
    if (record__update(&account, sizeof(account), ACCOUNT_DB, index) == -1 ||
        transaction_save_transaction(userId, -1, oldBalance, -1 * withdrawAmount, account.accountBalance) == -1)
    {
        send_message(fd, "Unable to process transaction");
        goto cleanup;
    }

    // Step 7: Show confirmation and new balance
    snprintf(buffer, sizeof(buffer),
             "\n\nWithdraw successful!\nPrevious Balance: %d\nWithdraw Amount: %d\nUpdated Balance: %d\n",
             oldBalance, withdrawAmount, account.accountBalance);
    send_message(fd, buffer);

    send_message(fd, "\n\nPress enter to continue...");
    receive_message(fd, &temp);

cleanup:
    free(temp);
    return showStartScreen();
}

// Transfer funds
void account_transfer_funds()
{
    char *temp = NULL;
    int fd = clientfd;
    int userId = logged_in_user.userId; // Sender’s user ID

    Account senderAccount, receiverAccount;
    int transferAmount = 0;
    int receiverId = 0;

    clear_terminal(fd);

    // Step 1: Search for the sender’s account
    int senderIndex = record__search(&senderAccount, sizeof(senderAccount), ACCOUNT_DB, __avb__cmp_userID, &userId);

    if (senderIndex == -1)
    {
        send_message(fd, "\nAccount not found.\n");
        sleep(2);
        goto cleanup;
    }

    // Step 2: Ask for receiver user ID
    send_message(fd, "\nEnter the account ID of the account to transfer funds to: ");
    receive_message(fd, &temp);
    receiverId = atoi(temp);
    // free(temp);

    // Step 3: Search for receiver account
    // TODO: cancel if receiver account is deactivated
    int receiverIndex = record__search(&receiverAccount, sizeof(receiverAccount), ACCOUNT_DB, __avb__cmp_userID, &receiverId);

    if (receiverIndex == -1)
    {
        send_message(fd, "\nAccount not found.\n");
        sleep(2);
        goto cleanup;
    }

    // Step 4: Ask for transfer amount
    send_message(fd, "\nEnter the amount to transfer: ");
    receive_message(fd, &temp);
    transferAmount = atoi(temp);
    // free(temp);

    if (transferAmount <= 0)
    {
        send_message(fd, "\nInvalid transfer amount.\n");
        sleep(2);
        goto cleanup;
    }

    // Step 5: Check sender’s balance
    if (senderAccount.accountBalance < transferAmount)
    {
        send_message(fd, "\nInsufficient balance.\n");
        sleep(2);
        goto cleanup;
    }

    // Step 6: Perform transfer
    int oldSenderBalance = senderAccount.accountBalance;
    int oldReceiverBalance = receiverAccount.accountBalance;

    senderAccount.accountBalance -= transferAmount;
    receiverAccount.accountBalance += transferAmount;

    // Step 7: Update both records in DB

    // transaction_save_transaction(receiverId, userId, oldReceiverBalance, transferAmount, receiverAccount.accountBalance);
    if (
        record__update(&senderAccount, sizeof(senderAccount), ACCOUNT_DB, senderIndex) == -1 ||
        record__update(&receiverAccount, sizeof(receiverAccount), ACCOUNT_DB, receiverIndex) == -1 ||
        transaction_save_transaction(userId, receiverId, oldSenderBalance, transferAmount, senderAccount.accountBalance) == -1)
    {
        send_message(fd, "Unable to process transaction");
        goto cleanup;
    }

    // Step 8: Display confirmation
    char buffer[512];
    //  "\n- Receiver (Account ID: %d) \n- Previous Balance: %d \n- New Balance: %d\n",
    snprintf(buffer, sizeof(buffer),
             "\n\nTransfer Successful!\n"
             "- Transferred Amount: %d\n"
             "- Sender (Account ID: %d) \n- Previous Balance: %d \n- New Balance: %d\n",
             transferAmount,
             senderAccount.accountId, oldSenderBalance, senderAccount.accountBalance,
             receiverAccount.accountId, oldReceiverBalance, receiverAccount.accountBalance);
    send_message(fd, buffer);

    send_message(fd, "\n\nPress enter to continue...");
    receive_message(fd, &temp);

cleanup:
    return showStartScreen();
    free(temp);
}
