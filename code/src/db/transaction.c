// transaction.c

//============================================================================

// This file contains logic and UI related to transactions

//============================================================================

#include "db/transaction.h"
#include "file_operation.h"
#include "start_screen.h"
#include "helper.h"
#include "config.h"
#include "db/transaction.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "communication.h"
#include "string.h"

// create and save transaction
int transaction_save_transaction(int from_uid, int to_uid, int prev_amount, int amount, int last_amount)
{
    Transaction transaction;
    transaction.transactionId = generateUniqueTransactionId();
    transaction.previousAmount = prev_amount;
    transaction.transactionAmount = amount;
    transaction.lastAmount = last_amount;
    transaction.from_uid = from_uid;
    transaction.to_uid = to_uid;

    return record__save(&transaction, sizeof(transaction), TRANSACTION_DB);
}

//============================================================================

// View user transaction

//============================================================================

static int __find_transaction_based_on_accountId(void *rec, void *ctx)
{
    Transaction *tempTransaction = (Transaction *)rec;
    int accountId = *(int *)ctx;

    return tempTransaction->from_uid == accountId || tempTransaction->to_uid == accountId ? 1 : 0;
}

static void __display_transactions(int fd, int count, Transaction *transactions, int accountId)
{
    clear_terminal(fd);
    send_message(fd, "\n=====================================\n");
    send_message(fd, "             My Transactions            \n");
    send_message(fd, "=====================================\n");
    send_message(fd, "\n\nTransactionId\t\tFrom\t\t\tTo\t\t\tAmount\t\t\tBalance Remaining");

    for (int i = 0; i < count; i++)
    {
        int transactionId = transactions[i].transactionId;
        int amount = transactions[i].transactionAmount;
        int from = transactions[i].from_uid;
        int to = transactions[i].to_uid;
        int finalBalance = transactions[i].lastAmount;

        send_message(fd, makeString("\n\n%d\t\t\t%d\t\t\t%d\t\t\t%d\t\t\t%d",
                                    transactionId, from, to,
                                    (to != -1 && to != accountId) ? -1 * amount : amount,
                                    finalBalance));
    }
}

// View user transaction
void transaction_view_transactions(int loggedUser)
{
    int fd = clientfd;
    char *accountIDChar = NULL;
    int accountId = logged_in_user.userId;
    void *transactionBits = NULL;

    // Get username to update
    if (loggedUser == 0)
    {
        clear_terminal(fd);
        if (prompt_user_input(fd, "\nEnter accountid of the customer: ", &accountIDChar) != 0)
            goto cleanup;

        accountId = atoi(accountIDChar);
    }

    int count = record__search_cont(&transactionBits, sizeof(Transaction), TRANSACTION_DB,
                                    &__find_transaction_based_on_accountId, &accountId);

    if (count == -1)
    {
        send_message(fd, "\n No Transactions found\n");
        goto cleanup;
    }

    Transaction *transactions = (Transaction *)transactionBits;

    __display_transactions(fd, count, transactions, accountId);
    waitTillEnter(fd);

cleanup:
    free(transactionBits);
    free(accountIDChar);
    return showStartScreen();
}