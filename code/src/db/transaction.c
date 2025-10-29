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

int __tvut_cmp(void *rec, void *ctx)
{
    Transaction *tempTransaction = (Transaction *)rec;
    int accountId = *(int *)ctx;

    return tempTransaction->from_uid == accountId || tempTransaction->to_uid == accountId ? 1 : 0;
}

static int __tvt_prompt_user_input(int fd, const char *message, char **out)
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
        send_message(fd, "\nOperation cancelled by user.\n");
        sleep(2);
        return -1;
    }

    return 0;
}

// view user transaction
void transaction_view_transactions(int loggedUser)
{
    int fd = clientfd;
    char *accountIDChar = NULL;
    int accountId = logged_in_user.userId;

    // Get username to update
    if (loggedUser == 0)
    {
        clear_terminal(fd);
        if (__tvt_prompt_user_input(fd, "\nEnter accountid of the customer: ", &accountIDChar) != 0)
            goto failure;

        accountId = atoi(accountIDChar);
    }

    void *transactionBits = NULL;
    int count = record__search_cont(&transactionBits, sizeof(Transaction), TRANSACTION_DB, &__tvut_cmp, &accountId);

    if (count == -1)
    {
        send_message(fd, "\n No Transactions found\n");
        goto failure;
    }

    Transaction *transactions = (Transaction *)transactionBits;

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

        char buffer[128];
        snprintf(buffer, sizeof(buffer),
                 "\n\n%d\t\t\t%d\t\t\t%d\t\t\t%d\t\t\t%d",
                 transactionId, from, to,
                 to != -1 && to != accountId ? -1 * amount : amount, finalBalance);

        send_message(fd, buffer);
    }

    char *tmp = NULL;
    send_message(fd, "\n\n\nPress enter to go back...");
    receive_message(fd, &tmp);
    free(tmp);

failure:
    free(transactionBits);
    free(accountIDChar);
    return showStartScreen();
}