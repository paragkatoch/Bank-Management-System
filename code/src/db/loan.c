// loan.c

//============================================================================

// This file contains logic and UI related to loans

//============================================================================
// TODO: loan amount to the account after approval
// TODO: at the time for deposit make from -1 and to accountid
#include "db/loan.h"
#include "config.h"
#include "helper.h"
#include "communication.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "file_operation.h"
#include "start_screen.h"
#include "db/account.h"
#include <unistd.h>
#include <fcntl.h>

static void __display_loans(int fd, int count, Loan *loansBits, char *header)
{
    Loan *loans = (Loan *)loansBits;

    clear_terminal(fd);
    send_message(fd, "\n=====================================\n");
    send_message(fd, makeString("               %s              \n", header));
    send_message(fd, "=====================================\n");

    if (count == 0)
    {
        // TODO: update
        send_message(fd, "\n\n== == == == No loans to display == == == ==\n\n");
        return;
    }

    send_message(fd, "\n\nIndex\t\t\tLoanId\t\t\tAccountId\t\tAmount\t\t\tStatus");

    for (int i = 0; i < count; i++)
    {
        int loanId = loans[i].loanId;
        int loanAmount = loans[i].loanAmount;
        int loanStatusInt = loans[i].loanStatus;
        int accountId = loans[i].accountID;

        char loanStatus[20];

        switch (loanStatusInt)
        {
        case LOAN_APPROVED:
            strcpy(loanStatus, "Approved");
            break;
        case LOAN_REJECTED:
            strcpy(loanStatus, "Rejected");
            break;
        case LOAN_PROCESSING:
            strcpy(loanStatus, "Processing");
            break;
        default:
            strcpy(loanStatus, "INVALID");
            break;
        }

        send_message(fd, makeString("\n%d\t\t\t%d\t\t\t%d\t\t\t%d\t\t\t%s",
                                    i + 1, loanId, accountId, loanAmount, loanStatus));
    }
}

static int __find_loan_based_on_loanId(void *rec, void *ctx)
{
    Loan *tempLoan = (Loan *)rec;
    int loanId = *(int *)ctx;

    return tempLoan->loanId == loanId ? 1 : 0;
}

//============================================================================

// Create and save new loan

//============================================================================

void loan_create_loan()
{
    int fd = clientfd;

    Loan newLoan;
    char *loanAmount = NULL;

    clear_terminal(fd);
    send_message(fd, "\n=====================================\n");
    send_message(fd, "     Loan Application Portal        \n");
    send_message(fd, "=====================================\n");

    send_message(fd, "\n Use -1 to cancel \n\n");

    // get updated details
    if (prompt_user_input(fd, "\nEnter loan amount: ", &loanAmount) != 0)
        goto cleanup;

    newLoan.loanId = generateUniqueLoanId();
    newLoan.loanAmount = atoi(loanAmount);
    newLoan.loanStatus = LOAN_PROCESSING; // 0 processed, 1 approved, 2 rejected
    newLoan.assignedID = LOAN_NOTASSIGNED;
    newLoan.accountID = logged_in_user.userId;

    clear_terminal(fd);

    // save user to db
    if (record__save(&newLoan, sizeof(Loan), LOAN_DB, RECORD_USE_LOCK) != 0)
        send_message(fd, "\n\nOops...something went wrong\n");
    else
        send_message(fd, "\n\nApplied for loan successfully.\n");

    waitTillEnter(fd);

    // cleanup
cleanup:
    free(loanAmount);
    return showStartScreen();
}

//============================================================================

// View loan status

//============================================================================

static int __find_loan_based_on_account_id(void *rec, void *ctx)
{
    Loan *tempLoan = (Loan *)rec;
    int accountId = *(int *)ctx;

    return tempLoan->accountID == accountId ? 1 : 0;
}

// View loan status
void loan_view_loan_status()
{
    int fd = clientfd;
    int accountId = logged_in_user.userId;

    void *loansBits = NULL;
    int count = record__search_cont(&loansBits, sizeof(Loan), LOAN_DB, &__find_loan_based_on_account_id, &accountId, RECORD_USE_LOCK);
    if (count == -1)
        goto cleanup;

    __display_loans(fd, count, loansBits, "My Loans");
    waitTillEnter(fd);

cleanup:
    free(loansBits);
    return showStartScreen();
}

//============================================================================

// View employee assigned loans

//============================================================================

typedef struct
{
    int userId;
    int loanStatus;
} userId_loanStatus;

static int __find_loans_based_on_userId_loanStatus(void *rec, void *ctx)
{
    Loan *tempLoan = (Loan *)rec;
    userId_loanStatus *temp_userId_loanStatus = (userId_loanStatus *)ctx;

    // return (tempLoan->assignedID == temp_userId_loanStatus->userId &&
    //         tempLoan->loanStatus == temp_userId_loanStatus->loanStatus)
    //            ? 1
    //            : 0;
    return (tempLoan->assignedID == temp_userId_loanStatus->userId &&
            temp_userId_loanStatus->loanStatus == tempLoan->loanStatus)
               ? 1
               : 0;
}

static void __display_a_loan(int fd, Loan *assignedLoans, int choice)
{
    clear_terminal(fd);
    send_message(fd, "\n=====================================\n");
    send_message(fd, makeString("              Loan (ID:%d)          \n", assignedLoans[choice - 1].loanId));
    send_message(fd, "=====================================\n\n\n");

    send_message(fd, makeString("Loan Id: %d\n\nAccount Id: %d\n\nLoan Amount: %d",
                                assignedLoans[choice - 1].loanId,
                                assignedLoans[choice - 1].accountID,
                                assignedLoans[choice - 1].loanAmount));
}

// View employee assigned loans
void loan_view_and_process_assigned_loans()
{
    int fd = clientfd;
    int userId = logged_in_user.userId;
    char *temp = NULL;
    void *assignedLoansBits = NULL;
    Loan *currentLoan = NULL;

    while (1)
    {
        // Find Processing Loans assigned to employee
        userId_loanStatus userId_and_loanStatus;
        userId_and_loanStatus.userId = userId;
        userId_and_loanStatus.loanStatus = LOAN_PROCESSING;

        assignedLoansBits = NULL;
        int count = record__search_cont(&assignedLoansBits, sizeof(Loan), LOAN_DB,
                                        &__find_loans_based_on_userId_loanStatus,
                                        &userId_and_loanStatus, RECORD_USE_LOCK);

        if (count == -1)
            goto cleanup;

        Loan *assignedLoans = (Loan *)assignedLoansBits;

        // Display loans to user
        __display_loans(fd, count, assignedLoans, "My Assigned Loans");

        // Get index to display details of a particular loan
        int choice;

        while (1)
        {
            if (prompt_user_input(fd, "\nEnter index number of the loan to review (-1 to go back): ", &temp) != 0)
            {
                goto cleanup;
            }

            choice = atoi(temp);

            if (choice < 1 || choice > count)
            {
                send_message(fd, "\nInvalid selection.\n");
                continue;
            }

            break;
        }

        // display a particular loan
        __display_a_loan(fd, assignedLoans, choice);

        // Aprrove, Reject or go back to view assigned loans
        currentLoan = &assignedLoans[choice - 1];
        int action;

        while (1)
        {
            if (prompt_user_input(fd, "\nEnter(-1 to go back) -\n\t1 - Approve\n\t2 - Reject\n: ", &temp) != 0)
            {
                currentLoan = NULL;
                break;
            }

            int choice = atoi(temp);

            if (choice == 1)
                action = LOAN_APPROVED;
            else if (choice == 2)
                action = LOAN_REJECTED;
            else
            {
                send_message(fd, "\nInvalid choice. Try again...\n");
                continue;
            }

            break;
        }

        // Go back to view assigned loans
        if (currentLoan == NULL)
        {
            continue;
        }

        if (currentLoan->loanStatus == LOAN_APPROVED)
        {
            int index;
            int lock_fd;
            Account account;

            if ((index = __find_Account_From_AccountId(fd, currentLoan->accountID, &account, RECORD_USE_LOCK)) == -1)
                goto cleanup;

            // lock record
            ssize_t size = sizeof(Account);
            lock_fd = lock_record_fd(ACCOUNT_DB, F_WRLCK, index * size, size);
            if (lock_fd == -1)
                goto cleanup;

            __find_Account_From_AccountId(fd, currentLoan->accountID, &account, RECORD_NOT_USE_LOCK);
            int oldBalance = account.accountBalance;
            account.accountBalance += currentLoan->loanAmount;

            // Save updated account back to database and Record the deposit transaction
            if (record__update(&account, size, ACCOUNT_DB, index, RECORD_NOT_USE_LOCK) == -1 ||
                transaction_save_transaction(-1, currentLoan->accountID, oldBalance, currentLoan->loanAmount, account.accountBalance) == -1)
            {
                send_message(fd, "\nUnable to process the loan");
                waitTillEnter(fd);
                goto cleanup;
            }

            (lock_fd != -1) && unlock_record(lock_fd, index * size, size);
        }

        int loanId = currentLoan->loanId;

        // process and save the loan
        int pos = record__search(currentLoan, sizeof(Loan), LOAN_DB, &__find_loan_based_on_loanId, &loanId, RECORD_USE_LOCK);
        currentLoan->loanStatus = action;

        if (record__update(currentLoan, sizeof(Loan), LOAN_DB, pos, RECORD_USE_LOCK) == -1)
        {
            send_message(fd, "\nUnable to process the loan\n");
        }
        else
        {
            send_message(fd, "\nLoan processed successfully");
        }

        waitTillEnter(fd);
    }

cleanup:
    free(assignedLoansBits);
    free(temp);
    free(currentLoan);
    return showStartScreen();
}

//============================================================================

// View and Assign non-assigned loans

//============================================================================

static int __find_non_assigned_loans(void *rec, void *ctx)
{
    Loan *tempLoan = (Loan *)rec;
    int non_assigned_loan = *(int *)ctx;

    return tempLoan->assignedID == non_assigned_loan ? 1 : 0;
}

// View and Assign non-assigned loans
void loan_view_non_assigned_loans_and_assign()
{
    int fd = clientfd;
    int non_assigned_loan = LOAN_NOTASSIGNED;
    char *temp = NULL;

    void *loansBits = NULL;

    while (1)
    {
        int count = record__search_cont(&loansBits, sizeof(Loan), LOAN_DB, &__find_non_assigned_loans, &non_assigned_loan, RECORD_USE_LOCK);

        if (count == -1)
            goto cleanup;

        Loan *loans = (Loan *)loansBits;
        __display_loans(fd, count, loansBits, "Non Assigned Loans");

        // Get index to display details of a particular loan
        int loanId;
        int userId;
        int choice;

        while (1)
        {
            if (prompt_user_input(fd, "\nEnter index number of the loan to assign (-1 to cancel): ", &temp) != 0)
            {
                goto cleanup;
            }

            choice = atoi(temp);

            if (choice < 1 || choice > count)
            {
                send_message(fd, "\nInvalid selection.\n");
                continue;
            }

            break;
        }

        loanId = loans[choice - 1].loanId;

        // get user id
        while (1)
        {
            if (prompt_user_input(fd, "\nEnter userId of the employee to assign loan to (-1 to cancel): ", &temp) != 0)
                goto cleanup;

            choice = atoi(temp);
            userId = choice;

            User tempUser;

            // check if userid belongs to employee
            int pos = find_user_based_on_userId(&tempUser, userId, RECORD_USE_LOCK);

            if (pos == -1 || tempUser.role != EMPLOYEE_ROLE)
            {
                send_message(fd, "\nInvalid userId\n");
                continue;
            }
            break;
        }

        // find loan and update it
        Loan tempLoan;
        int pos = record__search(&tempLoan, sizeof(Loan), LOAN_DB, &__find_loan_based_on_loanId, &loanId, RECORD_USE_LOCK);
        tempLoan.assignedID = userId;

        if (pos == -1 || record__update(&tempLoan, sizeof(Loan), LOAN_DB, pos, RECORD_USE_LOCK))
            send_message(fd, "\nUnable to assign the loan\n");
        else
            send_message(fd, "\nLoan Assigned successfully");
    }

cleanup:
    free(loansBits);
    free(temp);
    return showStartScreen();
}
